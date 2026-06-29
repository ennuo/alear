#include <Sync/Client.h>
#include <Sync/Cache.h>
#include <Sync/Serialise.h>
#include <Sync/Bootstrap.h>
#include <netdb.h>
#include <JobManager.h>
// #include <Crafteroids.h>
#include <GetLoginCookieTask.h>
#include <ResourceSystem.h>
#include <ResourceFileOfBytes.h>
#include <map>
#include <fifo.h>
#include <Fart.h>
#include <FartRO.h>
#include <np.h>
#include <Directory.h>
#include <network/NetworkManager.h>
#include <network/RNPManager.h>
#include <AlearStartMenu.h>

#include <mem_stl_buckets.h>

#define SYNC_ERROR(...) MMLogCh(DC_NETWORK, "sync: " __VA_ARGS__); Disconnect()

namespace sync
{
    CClient* Client;
    CJobManager* DownloadJobManager;

    typedef std::map<u32, MessageCallback, std::less<u32>, STLBucketAlloc<std::pair<u32, MessageCallback> > > CallbackMap;
    static CallbackMap RegisteredCallbacks;
    static CRawVector<StateChangeCallback> RegisteredStateCallbacks;

    void RegisterStateChangeCallback(StateChangeCallback cb)
    {
        RegisteredStateCallbacks.push_back(cb);
    }

    void RegisterMessageCallback(int channel, MessageCallback cb)
    {
        RegisteredCallbacks.insert(CallbackMap::value_type(channel, cb));
    }

    bool IsNetworkReady()
    {
        int state;
        if (sceNpManagerGetStatus(&state) != CELL_OK)
            return false;

        if (state == SCE_NP_MANAGER_STATUS_OFFLINE)
            return Config.GetString("username") != NULL;

        return state == SCE_NP_MANAGER_STATUS_ONLINE;
    }

    bool unbuffered_send(SOCKET s, void* data, int len)
    {
        int bytes_sent = 0;
        while (bytes_sent < len)
        {
            int ret = send(s, (void*)((char*)data + bytes_sent), len - bytes_sent, 0);
            if (ret <= 0) return false;
            bytes_sent += ret;
        }
        
        return true;
    }

    bool recv_exactly(SOCKET s, char* buf, int len)
    {
        while (len > 0)
        {
            int ret = recv(s, buf, len, 0);
            if (ret <= 0) return false;

            len -= ret;
            buf += ret;
        }

        return len == 0;
    }

    bool recvmsg(SOCKET s, packet& p, ByteArray& data)
    {
        if (!recv_exactly(s, (char*)&p, sizeof(packet))) return false;

        u32 length = p.Length - sizeof(packet);
        data.resize(length);

        if (length > 0)
            return recv_exactly(s, data.begin(), length);
        
        return true;
    }

    bool tryrecvmsg(SOCKET s, packet& p, ByteArray& data, bool& listen_error)
    {
        timeval tv = {0};
        fd_set readfds, errorfds;
        int ret;

        FD_ZERO(&readfds);
        FD_ZERO(&errorfds);
        FD_SET(s, &readfds);
        FD_SET(s, &errorfds);

        ret = select(FD_SETSIZE, &readfds, NULL, &errorfds, &tv);
        if (ret == 0) return false;
        if (ret < 0 || FD_ISSET(s, &errorfds))
        {
            listen_error = true;
            return false;
        }

        if (!FD_ISSET(s, &readfds)) return false;

        return recvmsg(s, p, data);
    }

    template <typename T>
    bool parsemsg(ByteArray& data, T& msg)
    {
        CReflectionLoadVector r(&data);
        r.SetCompressionFlags(0);
        r.SetRevision(SRevision(kProtocolVersion));

        return Reflect(r, msg) == REFLECT_OK;
    }

    template <typename T>
    bool recvmsg(SOCKET s, T& msg)
    {
        packet p;
        ByteArray data;
        if (!recvmsg(s, p, data)) return false;

        if (p.Message != GetMessageType<T>() || p.Channel != GetChannel<T>())
        {
            SYNC_LOG("recv'd unexpected message in response\n");
            return false;
        }

        if (p.Status != kResponse_Ok)
        {
            SYNC_LOG("msg had failed status (%04x)\n", p.Status);
            return false;
        }
        
        return parsemsg(data, msg);
    }

    bool sendmsg(SOCKET s, u32 channel, u32 message, void* data, int len)
    {
        packet p;
        p.Length = sizeof(packet) + len;
        p.Message = message;
        p.Channel = channel;
        p.Status = kResponse_Ok;
        
        if (!unbuffered_send(s, &p, sizeof(packet))) return false;
        if (len > 0)
            return unbuffered_send(s, data, len);

        return true;
    }

    template <typename T>
    bool sendmsg(SOCKET s, T& msg)
    {
        CSyncSaveVector r;
        if (Reflect(r, msg) != REFLECT_OK)
        {
            SYNC_LOG("failed to serialize message, errno: xxx\n");
            return false;
        }

        return sendmsg(s, GetChannel<T>(), GetMessageType<T>(), r.Data.begin(), r.Data.size());
    }

    bool sendmsg(SOCKET s, u32 channel, u32 message)
    {
        return sendmsg(s, channel, message, NULL, 0);
    }

    void OnDownloadFinish(const CP<CSerialisedResource>& csr, int priority, int state)
    {
        switch (state)
        {
            case kDownloadState_Success:
            {
                AddCSRToDoneQueue(csr, priority);
                break;
            }
            case kDownloadState_NetworkError:
            case kDownloadState_NoDataSource:
            {
                if (!gNetworkManager.ConnectionManager.IsDownloadOK())
                {
                    SetResourceError(csr, LOAD_STATE_ERROR_NO_DATA_SOURCE);
                    break;
                }

                // Resources that prefer to be loaded as a loose file
                // can't be loaded over either the HTTP or RNP server.
                if (GetPreferredSerialisationType(csr->GetDescriptor().GetType()) == PREFER_FILE)
                {
                    SetResourceError(csr, LOAD_STATE_ERROR_NO_DATA_SOURCE);
                    break;
                }

                // If we have no data source, we can now try
                // passing the resource to either the RNP or HTTP
                // thread depending on which is available.
                if (gRNPManager.CanHandleCSR())
                {
                    AddCSRToQueue(csr, CSRsForRNP, priority);
                    break;
                }

                // Make sure the resource we're trying to download actually has a hash
                // to pass to the HTTP resource thread.
                CHash hash = csr->GetDescriptor().LatestHash();
                if (hash)
                {
                    AddCSRToQueue(csr, CSRsForHTTP, priority);
                    break;
                }

                SetResourceError(csr, LOAD_STATE_ERROR_NO_DATA_SOURCE);
                break;
            }
            case kDownloadState_InvalidSession:
            case kDownloadState_BadHash:
            {
                // requeue the download for bad hash
                // request a new session for invalid session
                SetResourceError(csr, LOAD_STATE_ERROR_NO_DATA_SOURCE);
                break;
            }
            default:
            {
                SetResourceError(csr, LOAD_STATE_ERROR_NO_DATA_SOURCE);
                break;
            }
        }
    }

    CDownloadJob::CDownloadJob() : CBaseCounted(),
    CSR(), Server(), Token(), DownloadSize(),
    BytesDownloaded(), JobID(), State(kDownloadState_Inactive), Priority(STREAM_NO_STREAMING)
    {
        Reset();
    }

    CUploadJob::CUploadJob() : CBaseCounted(),
    Source(), Server(), Token(), UploadSize(), BytesUploaded(), JobID(), State(kUploadState_Inactive)
    {

    }


    void CDownloadJob::Reset()
    {
        CSR = NULL;
        memset(&Server, 0, sizeof(sockaddr_in));
        memset(&Token, 0, sizeof(Token));
        DownloadSize = 0;
        BytesDownloaded = 0;
        JobID = 0;
        State = kDownloadState_Inactive;
    }

    void CDownloadJob::EnqueueForDownload(const CP<CSerialisedResource>& csr, int priority)
    {
        Reset();

        CSR = csr;
        Priority = priority;

        const CResourceDescriptorBase& desc = csr->GetDescriptor();
        if (GetPreferredSerialisationType(desc.GetType()) != PREFER_FILE)
        {
            u32 size;
            if (gCaches[CT_SYNC]->GetSize(desc.GetHash(), size))
            {
                State = kDownloadState_Success;
                Finish();
                return;
            }
        }
        else
        {
            CFileDBRow* row = FileDB::FindByGUID(desc.GetGUID());
            if (row != NULL) 
            {
                csr->LoosePath = CFilePath(FPR_GAMEDATA, row->FilePathX);
            }
            else
            {
                State = kDownloadState_NoDataSource;
                Finish();
                return;
            }
        }

        Server = Client->GetServerAddress();
        Token = Client->GetSession();
        State = kDownloadState_InProgress;

        JobID = DownloadJobManager->EnqueueJob(
            1000, &DownloadJob, (void*)this,
            Client->GetDownloadTag(), "DownloadJob"
        );
    }

    void CDownloadJob::Finish()
    {
        if (!CSR)
        {
            Reset();
            return;
        }

        OnDownloadFinish(CSR, Priority, State);
        Reset();
    }

    class CUploadClient {
    public:
        CUploadClient(CUploadJob* owner) : Socket(-1), Owner(owner)
        {

        }

        ~CUploadClient()
        {
            if (Socket != -1)
            {
                shutdown(Socket, SHUT_RDWR);
                close(Socket);
                Socket = -1;
            }
        }

        void SetState(int state)
        {
            Owner->State = state;
        }

        bool Connect()
        {
            Socket = socket(AF_INET, SOCK_STREAM, IPPROTO_IP);
            if (Socket == -1)
            {
                SetState(kUploadState_NetworkError);
                return false;
            }

            if (connect(Socket, (sockaddr*)&Owner->Server, sizeof(sockaddr_in)) != 0)
            {
                SetState(kUploadState_NetworkError);
                return false;
            }

            return true;
        }

        bool Work()
        {
            CFileSource& source = Owner->Source;
            msg_upload msg;
            msg.FileSize = Owner->UploadSize;
            msg.Hash = source.Hash;
            msg.Token = Owner->Token;

            if (!sendmsg(Socket, msg))
                return false;

            SResourceReader reader;
            int fd;
            bool loose = false;

            if (!source.IsSlow())
            {
                if (!GetResourceReader(source.Hash, reader))
                {
                    SetState(kUploadState_NoDataSource);
                    return false;
                }

                fd = reader.Handle;
            }
            else
            {
                if (!FileExists(source.FilePath))
                {
                    SetState(kUploadState_NoDataSource);
                    return false;
                }

                if (!FileOpen(source.FilePath, fd, OPEN_READ))
                {
                    SetState(kUploadState_IOError);
                    return false;
                }

                loose = true;
            }


            char buf[1024];
            while (Owner->BytesUploaded < Owner->UploadSize)
            {
                u64 n = FileRead(fd, buf, MIN(sizeof(buf), Owner->UploadSize - Owner->BytesUploaded));
                if (!unbuffered_send(Socket, buf, n))
                {
                    SetState(kUploadState_NetworkError);
                    if (loose) FileClose(fd);
                    return false;

                }

                Owner->BytesUploaded += n;
            }

            if (loose) FileClose(fd);

            packet p;
            ByteArray data;
            if (!recvmsg(Socket, p, data))
                return false;

            if (p.Status != kResponse_Ok)
                return false;
            
            return true;
        }

        int Socket;
        CUploadJob* Owner;
        msg_download_response Response;
    };

    struct CDownloadClient
    {
        CDownloadClient(void* owner) : Socket(-1), Owner((CDownloadJob*)owner)
        {

        }

        ~CDownloadClient()
        {
            if (Socket != -1)
            {
                shutdown(Socket, SHUT_RDWR);
                close(Socket);
                Socket = -1;
            }
        }

        void SetState(int state)
        {
            Owner->State = state;
        }

        bool Connect()
        {
            Socket = socket(AF_INET, SOCK_STREAM, IPPROTO_IP);
            if (Socket == -1)
            {
                SetState(kDownloadState_NetworkError);
                return false;
            }

            if (connect(Socket, (sockaddr*)&Owner->Server, sizeof(sockaddr_in)) != 0)
            {
                SetState(kDownloadState_NetworkError);
                return false;
            }

            return true;
        }

        bool StreamToFile()
        {
            FileHandle fd;
            const CFilePath& fp = Owner->CSR->LoosePath;

            MMLog("streaming file %s\n", fp.c_str());
            
            DirectoryCreate(fp);
            if (!FileOpen(fp, fd, OPEN_WRITE))
            {
                MMLog("failed to stream file!\n");
                return false;
            }

            const int kBufferSize = 4096;
            CSHA1Context sha1;
            char buf[kBufferSize];

            int n = 0;
            while (n < Response.FileSize)
            {
                int size = MIN(Response.FileSize - n, kBufferSize);
                bool rv = recv_exactly(Socket, buf, size);
                if (!rv)
                {
                    FileClose(fd);
                    FileUnlink(fp);
                    return false;
                }

                sha1.AddData((const u8*)buf, size);
                FileWrite(fd, (void*)buf, size);
                n += size;
            }

            FileClose(fd);

            CHash hash;
            sha1.Result((u8*)&hash);
            if (hash != Response.FileHash)
            {
                SetState(kDownloadState_BadHash);
                FileUnlink(fp);
                return false;
            }

            return true;
        }

        bool StreamToCache(const ByteArray& data)
        {
            return gCaches[CT_SYNC] != NULL ? gCaches[CT_SYNC]->Put(Response.FileHash, data.begin(), Response.FileSize) : false;
        }

        bool Work()
        {
            CResourceDescriptorBase desc = Owner->CSR->GetDescriptor();
            msg_download rqst;
            rqst.GUID = desc.GetGUID();
            rqst.Hash = desc.LatestHash();
            rqst.Token = Owner->Token;

            if (!sendmsg(Socket, rqst))
            {
                SYNC_LOG("failed to send download request to server\n");
                return false;
            }

            if (!recvmsg(Socket, Response))
            {
                SYNC_LOG("failed to receive download response from server\n");
                return false;
            }
            
            SYNC_LOG("download worker got fileinfo size=%d, hash=%s, is_compressed=%s", Response.FileSize, StringifyHash(Response.FileHash).c_str(), Response.IsCompressed ? "true" : "false");

            if (Response.IsCompressed)
            {
                SYNC_LOG("compressed data isn't supported in download responses!\n");
                return false;
            }
            else
            {
                if (GetPreferredSerialisationType(desc.GetType()) == PREFER_FILE)
                    return StreamToFile();
                else
                {
                    ByteArray data;
                    data.resize(Response.FileSize);
                    if (!recv_exactly(Socket, data.begin(), data.size()))
                    {
                        SYNC_LOG("an error occurred in worker while receiving file data!\n");
                        return false;
                    }

                    if (!StreamToCache(data))
                    {
                        SYNC_LOG("failed to stream to cache\n");
                        return false;
                    }

                    Owner->CSR->Data.swap(data);
                    return true;
                }
            }

            return false;
        }

        int Socket;
        CDownloadJob* Owner;
        msg_download_response Response;
    };

    void CDownloadJob::DownloadJob(void* userdata)
    {
        CDownloadJob* d = (CDownloadJob*)userdata;
        CDownloadClient client(userdata);

        if (!client.Connect()) return;

        if (!client.Work())
        {
            SYNC_LOG("DOWNLOAD FAILED!!! (%d)\n", d->State);
            if (d->State < kDownloadState_Failed)
                d->State = kDownloadState_NoDataSource;
        }
        else
        {
            d->State = kDownloadState_Success;
            SYNC_LOG("DOWNLOAD SUCCESS!!!\n");
        }

        d->Finish();
    }

    void CUploadJob::UploadJob(void* userdata)
    {
        CUploadJob* d = (CUploadJob*)userdata;
        CUploadClient c(d);

        if (!c.Connect()) return;

        if (!c.Work())
        {
            if (d->State < kUploadState_Failed) 
                d->State = kUploadState_Failed;
        }
        else d->State = kUploadState_Success;
    }

    CP<CUploadJob> CUploadJob::EnqueueForUpload(const CFileSource& source)
    {
        CP<CUploadJob> job = new CUploadJob();

        job->Source = source;
        job->Server = Client->GetServerAddress();
        job->Token = Client->GetSession();

        if (!source.IsSlow())
        {
            SYNC_LOG("Uploading h%s\n", StringifyHash(source.Hash).c_str());
            for (u32 i = 0; i < CT_COUNT; ++i)
            {
                if (gCaches[i] != NULL && gCaches[i]->GetSize(source.Hash, job->UploadSize))
                    break;
            }
        }
        else
        {
            SYNC_LOG("Uploading h%s (%s)\n", StringifyHash(source.Hash).c_str(), source.FilePath.c_str());
            u64 modtime, size;
            FileStat(source.FilePath, modtime, size);
            job->UploadSize = size;
        }

        job->BytesUploaded = 0;
        
        job->JobID = DownloadJobManager->EnqueueJob(
            1000, &UploadJob, (void*)job.GetRef(),
            Client->GetUploadTag(), "UploadJob"
        );
        job->State = kUploadState_InProgress;

        return job;
    }

    CCommitTask::CCommitTask(const CFilePath& fp, u64 depot_id) : CBaseCounted(),
    Depot(depot_id), State(kState_Inactive), UploadTask(), DatabaseFilePath(fp), CommitMessage()
    {
        CommitMessage.DepotID = Depot;
    }

    void DownloadTest(void* userdata)
    {
        SYNC_LOG("download test, making sure tag is correct!!!: %08x\n", ((CClient*)userdata)->DownloadTag);
    }

    CClient::CClient() : WantQuit(false), WantConnect(true),
    State(kState_Disconnected), SubState(kSubState_None), Socket(-1), Address(), Thread(),
    DownloadMutex("DownloadMutex"), UploadMutex("UploadMutex"), DownloadTag(), DownloadsInProgress(),
    ServerInfo(), UploadTask(), PendingDepotRequests(),
    CommitMutex("CommitMutex"), CommitTask()
    {
        // Using the HTTP job manager because it's probably going to be
        // less congested than the main job manager.
        DownloadTag = DownloadJobManager->GetUniqueTag();
        UploadTag = DownloadJobManager->GetUniqueTag();
        SYNC_LOG("constructed sync client, download tag = 0x%08x, test download job = 0x%08x, sizeof(packet)=0x%08x\n", DownloadTag,
            DownloadJobManager->EnqueueJob(1000, &DownloadTest, (void*)this, DownloadTag, "Download Test"), sizeof(packet)
        );

        Thread = ThreadCreate(&ThreadFunctionStatic, (u64)this, "alear sync client thread");
    }

    CClient::~CClient()
    {
        WantQuit = true;
        Disconnect();
        if (Thread != NULL)
            ThreadJoin(Thread);
    }

    MAKE_THREAD_FUNCTION(CClient::ThreadFunctionStatic)
    {
        CClient* client = (CClient*)arg;

        while (!client->WantQuit)
        {

            if (client->State == kState_Disconnected)
            {
                if (client->WantConnect && IsNetworkReady())
                {
                    client->Connect(Config.GetString("address", "localhost"), Config.GetInt("port", kSyncServerPort));
                    client->WantConnect = false;
                }
            }

            client->Update();
            ThreadSleep(33);
        }
        
        THREAD_RETURN(0);
    }

    bool CClient::Connect(const char* host, int port)
    {
        SYNC_LOG("connecting to %s:%d\n", host, port);

        memset(&Address, 0, sizeof(sockaddr_in));
        Address.sin_port = port;
        Address.sin_family = AF_INET;

        hostent* hp = gethostbyname(host);
        if (hp == NULL)
        {
            SYNC_LOG("failed to gethostbyname %s\n", host);
            return false;
        }

        if (hp->h_addrtype != AF_INET)
        {
            SYNC_LOG("failed to get IPv4 address for host %s\n", host);
            return false;
        }

        memcpy(&Address.sin_addr, hp->h_addr, hp->h_length);


        Socket = socket(AF_INET, SOCK_STREAM, IPPROTO_IP);
        if (Socket == -1)
        {
            SYNC_LOG("failed to create socket for sync server!\n");
            return false;
        }

        if (connect(Socket, (sockaddr*)&Address, sizeof(sockaddr_in)) != 0)
        {
            SYNC_LOG("failed to connect to sync server!\n");
            return false;
        }

        SYNC_LOG("connection successful!\n");
        
        State = kState_Connecting;
        SubState = kSubState_None;

        return true;
    }

    void CClient::Disconnect()
    {
        SYNC_LOG("disconnecting...\n");

        for (StateChangeCallback* cb = RegisteredStateCallbacks.begin(); cb != RegisteredStateCallbacks.end(); ++cb)
            (*cb)(this, kState_Disconnected);

        if (State != kState_Error)
        {
            State = kState_Disconnected;
            SubState = kSubState_None;
        }

        if (Socket != -1)
        {
            shutdown(Socket, SHUT_RDWR);
            close(Socket);
        }

        Socket = -1;
    }

    void CClient::Download(CP<CSerialisedResource>& csr, int priority)
    {
        bool printed = false;
        while (IsConnecting())
        {
            if (!printed)
            {
                SYNC_LOG("waiting for sync server to come up before queueing downloading!\n");
                printed = true;
            }

            ThreadSleep(33);
        }

        if (!IsConnected())
        {
            OnDownloadFinish(csr, priority, kDownloadState_NetworkError);
            return;
        }

        CP<CDownloadJob> dl = new CDownloadJob();
        CCSLock lock(&DownloadMutex, __FILE__, __LINE__);
        DownloadsInProgress.push_back(dl);
        dl->EnqueueForDownload(csr, priority);
    }

    bool CClient::SendMessage(u32 channel, u32 message, void* data, int size)
    {
        if (!sendmsg(Socket, channel, message, data, size))
        {
            SYNC_ERROR("failed to send message to server!\n");
            return false;
        }

        return true;
    }
    
    void CClient::HandleSyncChannel(const packet& packet, ByteArray& data)
    {
        switch (packet.Message)
        {
            case eMessageType_Commit:
            {
                CCSLock _lock(&CommitMutex, __FILE__, __LINE__);
                if (!CommitTask || CommitTask->State != CCommitTask::kState_Committing)
                {
                    SYNC_ERROR("Got commit response despite no pending commit task!\n");
                    return;
                }

                if (packet.Status != kResponse_Ok)
                {
                    SYNC_LOG("Commit failed!\n");
                    CommitTask->State = CCommitTask::kState_Failed;
                    return;
                }

                CommitTask->State = CCommitTask::kState_Success;
                break;
            }

            case eMessageType_Depot:
            {
                CCSLock _lock(&DepotMutex, __FILE__, __LINE__);

                if (packet.Status != kResponse_Ok)
                {
                    SYNC_ERROR("failed to fetch depot!\n");
                    return;
                }

                msg_depot_response resp;
                if (!parsemsg(data, resp))
                {
                    SYNC_ERROR("failed to parse depot reply from server!\n");
                    return;
                }

                depot* depot = GetDepot(resp.DepotID);
                depot->Flags |= eDepotFlags_WantReload;
                CFilePath fp = depot->MakeDatabaseFilePath();
                DirectoryCreate(fp);
                
                int fd;
                if (FileOpen(fp, fd, OPEN_WRITE))
                {
                    FileWrite(fd, resp.FileData.begin(), resp.FileData.size());
                    FileClose(fd);
                }

                break;

            }

            case eEventType_Commit:
            {
                CCSLock _lock(&DepotMutex, __FILE__, __LINE__);

                commit_info info;
                if (!parsemsg(data, info))
                {
                    SYNC_ERROR("Failed to parse commit event from server!\n");
                    return;
                }

                const depot* depot = GetDepot(info.DepotId);
                if (depot == NULL)
                {
                    SYNC_LOG("Ignoring commit event since depot doesn't exist locally!\n");
                    return;
                }

                if (depot->IsDisabled())
                {
                    SYNC_LOG("Ignoring commit event since depot is disabled!\n");
                    return;
                }

                sendmsg(Socket, eChannel_Sync, eMessageType_Depot, (void*)&depot->DepotID, sizeof(u64));
                SYNC_LOG("got commit for %s with %d changes, %d additions, %d deletions\n", depot->Name, info.FilesChanged, info.FilesAdded, info.FilesDeleted);

                break;
            }
        }
    }

    void CClient::HandleGateChannel(const packet& packet, ByteArray& data)
    {
        switch (packet.Message)
        {

        }
    }

    void CClient::HandleResourceChannel(const packet& packet, ByteArray& data)
    {
        switch (packet.Message)
        {
            case eMessageType_FilterResources:
            {
                if (!UploadTask || UploadTask->State != CUploadTask::kState_Filtering)
                {
                    SYNC_ERROR("Got filtered resources response despite no pending upload task!\n");
                    return;
                }

                UploadTask->State = CUploadTask::kState_FinishedError;

                if (packet.Status != kResponse_Ok)
                {
                    SYNC_LOG("Filter resources request failed!\n");
                    return;
                }

                msg_resource_list resp;
                if (!parsemsg(data, resp))
                {
                    SYNC_ERROR("Failed to parse filtered resources response from server!\n");
                    return;
                }

                UploadTask->Jobs.reserve(resp.Hashes.size());
                UploadTask->Hashes.swap(resp.Hashes);
                UploadTask->State = CUploadTask::kState_Filtered;

                break;
            }
        }
    }

    void CClient::UpdateSynced()
    {
        CCSLock lock(&DepotMutex, __FILE__, __LINE__);
        for (u32 i = 0; i < Depots.size(); ++i)
        {
            depot& d = Depots[i];
            if (!d.WantReload()) continue;

            CFileDB* database = new CFileDB(d.MakeDatabaseFilePath());
            database->Load();

            if (d.Database != NULL) delete d.Database;
            d.Database = database;
            d.Flags &= ~eDepotFlags_WantReload;

            ReloadModifiedResources(database, NULL);
        }
    }

    void CClient::Update()
    {
        if (State == kState_Disconnected || State == kState_Error) return;
        
        packet p;
        ByteArray data;

        bool listen_error = false;
        if (tryrecvmsg(Socket, p, data, listen_error))
        {
            if (State == kState_Connected)
            {
                switch (p.Channel)
                {
                    case eChannel_Gate: HandleGateChannel(p, data); break;
                    case eChannel_Sync: HandleSyncChannel(p, data); break;
                    case eChannel_Resource: HandleResourceChannel(p, data); break;
                    default:
                    {
                        CallbackMap::iterator it = RegisteredCallbacks.find(p.Channel);
                        if (it == RegisteredCallbacks.end())
                        {
                            SYNC_LOG("recv unhandled channel, ignoring\n");
                            break;
                        }

                        it->second(this, p, data);
                        break;
                    }
                }
            }
            else if (State == kState_Connecting)
            {
                switch (SubState)
                {
                    case kSubState_WaitingForServerInfo:
                    {
                        if (p.Message != eMessageType_ServerInfo)
                        {
                            SYNC_ERROR("expected server info on client wakeup!\n");
                            return;
                        }

                        if (!parsemsg(data, ServerInfo))
                        {
                            SYNC_ERROR("failed to parse server info reply from the server!\n");
                            return;
                        }

                        SYNC_LOG("ServerInfo:\n");
                        SYNC_LOG("\t%s->%d\n", "ProtocolVersion", ServerInfo.ProtocolVersion);
                        SYNC_LOG("\t%s->%s\n", "Name", ServerInfo.Name.c_str());
                        SYNC_LOG("\t%s->%s\n", "Version", ServerInfo.Version.c_str());
                        SYNC_LOG("\t%s->%s\n", "BuildDate", ServerInfo.BuildDate.c_str());
                        for (u32 i = 0; i < ServerInfo.Extensions.size(); ++i)
                        {
                            extension_info& ext = ServerInfo.Extensions[i];
                            SYNC_LOG("\t\t%s\n", ext.Id.c_str());
                        }

                        if (ServerInfo.ProtocolVersion != kProtocolVersion)
                        {
                            SYNC_ERROR("protocol version mismatch: %d != %d\n", kProtocolVersion, ServerInfo.ProtocolVersion);
                            return;
                        }

                        msg_login login;
                        {
                            CCSLock nplock(&gCookieSec, __FILE__, __LINE__);
                            if (GetNPTicket())
                            {
                                login.Method = eAuthenticationType_Ticket;
                                login.Ticket = gNPTicket;
                            }
                            else
                            {
                                const char* username = Config.GetString("username");
                                const char* password = Config.GetString("password");

                                if (username != NULL && password != NULL)
                                {
                                    login.Method = eAuthenticationType_Credentials;
                                    login.Username = username;
                                    login.Password = password;
                                }
                                else
                                {
                                    SYNC_ERROR("failed to authenticate against sync server!\n");
                                }
                            }
                        }

                        if (!sendmsg(Socket, login))
                        {
                            SYNC_ERROR("failed to send login message to server!\n");
                            return;
                        }

                        SYNC_LOG("sent login to server, waiting for reply!\n");

                        SubState = kSubState_WaitingForLogin;
                        break;
                    }
                    case kSubState_WaitingForLogin:
                    {
                        if (p.Message != eMessageType_Login)
                        {
                            SYNC_ERROR("expected login response from server!\n");
                            return;
                        }

                        if (p.Status != kResponse_Ok)
                        {
                            SYNC_ERROR("failed to login!\n");
                            return;
                        }

                        msg_login_response resp;
                        if (!parsemsg(data, resp))
                        {
                            SYNC_ERROR("failed to parse login reply from server!\n");
                            return;
                        }

                        SYNC_LOG("successfully logged into alear server\n");

                        AccountID = resp.AccountID;
                        Token = resp.Token;

                        SYNC_LOG("token bytes: %08x\n", *(uint32_t*)&resp.Token);

                        if (!sendmsg(Socket, eChannel_Sync, eMessageType_DepotList))
                        {
                            SYNC_ERROR("failed to send request for depot list!\n");
                            return;
                        }

                        SubState = kSubState_WaitingForDepotList;

                        break;
                    }
                    case kSubState_WaitingForDepotList:
                    {
                        if (p.Message != eMessageType_DepotList)
                        {
                            SYNC_ERROR("expected depot list response from server!\n");
                            return;
                        }

                        if (p.Status != kResponse_Ok)
                        {
                            SYNC_ERROR("failed to fetch depot list!\n");
                            return;
                        }

                        CVector<depot> server_depots;
                        if (!parsemsg(data, server_depots))
                        {
                            SYNC_ERROR("failed to parse depot list reply from server!\n");
                            return;
                        }

                        PendingDepotRequests = 0;


                        CCSLock lock(&DepotMutex, __FILE__, __LINE__);
                        SYNC_LOG("Depots:\n");
                        for (u32 i = 0; i < server_depots.size(); ++i)
                        {
                            depot& d = server_depots[i];

                            const depot* c = GetDepot(d.DepotID);

                            bool download = c == NULL || c->CommitID != d.CommitID || !FileExists(c->MakeDatabaseFilePath());
                            if (d.IsBranched() && !IsBranchDefined(d.Branch))
                                d.Flags |= eDepotFlags_Disabled;

                            if (d.IsDisabled())
                                continue;
                            
                            if (download)
                            {
                                sendmsg(Socket, eChannel_Sync, eMessageType_Depot, (void*)&d.DepotID, sizeof(u64));
                                PendingDepotRequests++;
                            }

                            SYNC_LOG("%s\n", d.Id);

                            SYNC_LOG("srv[%s] %s, prio = %d, download = %s\n", d.Id, d.Name, d.Priority, download ? "true" : "false");
                        }


                        // Remove all server depots from the list and re-add the newly
                        // received list.
                        for (depot* it = Depots.begin(); it != Depots.end(); )
                        {
                            if (it->IsRemote())
                            {
                                if (it->Database)
                                {
                                    delete it->Database;
                                    it->Database = NULL;
                                }

                                it = Depots.erase(it);
                            }
                            else it++;
                        }

                        Depots.reserve(Depots.size() + server_depots.size());
                        for (u32 i = 0; i < server_depots.size(); ++i)
                            Depots.push_back(server_depots[i]);
                        
                        SaveToDepotCache();
                        SubState = kSubState_WaitingForDepotDownloads;

                        break;
                    }
                    case kSubState_WaitingForDepotDownloads:
                    {
                        if (p.Message != eMessageType_Depot)
                        {
                            SYNC_ERROR("expected depot response from server!\n");
                            return;
                        }

                        if (p.Status != kResponse_Ok)
                        {
                            SYNC_ERROR("failed to fetch depot!\n");
                            return;
                        }

                        SYNC_LOG("got depot message\n");

                        msg_depot_response resp;
                        if (!parsemsg(data, resp))
                        {
                            SYNC_ERROR("failed to parse depot reply from server!\n");
                            return;
                        }

                        const depot* depot = GetDepot(resp.DepotID);
                        CFilePath fp = depot->MakeDatabaseFilePath();
                        DirectoryCreate(fp);

                        int fd;
                        if (FileOpen(fp, fd, OPEN_WRITE))
                        {
                            FileWrite(fd, resp.FileData.begin(), resp.FileData.size());
                            FileClose(fd);
                        }

                        PendingDepotRequests--;
                        break;
                    }
                }
            }
        }
        else if (listen_error)
        {
            SYNC_ERROR("failed to recv msg from server, disconnected\n");
            return;
        }

        if (State == kState_Connecting)
        {
            switch (SubState)
            {
                case kSubState_None:
                {
                    SYNC_LOG("requesting server information\n");
                    sendmsg(Socket, eChannel_Gate, eMessageType_ServerInfo);
                    SubState = kSubState_WaitingForServerInfo;
                    break;
                }
                case kSubState_WaitingForDepotDownloads:
                {
                    if (PendingDepotRequests == 0)
                        SubState = kSubState_FinishedDownloadingDepots;
                    break;
                }
                case kSubState_FinishedDownloadingDepots:
                {
                    LinkDepots();

                    for (StateChangeCallback* cb = RegisteredStateCallbacks.begin(); cb != RegisteredStateCallbacks.end(); ++cb)
                        (*cb)(this, kState_Connected);

                    State = kState_Connected;
                    SubState = kSubState_None;

                    break;
                }
            }
        }

        if (State == kState_Connected)
        {
            UpdateUploadTasks();
            UpdateCommitTask();
            UpdateDownloadTasks();
        }
    }

    CP<CUploadTask> CClient::Upload(const CRawVector<CFileSource>& sources)
    {
        if (sources.size() == 0) return NULL;

        CCSLock _lock(&UploadMutex, __FILE__, __LINE__);
        
        CP<CUploadTask> task = UploadTask;
        while (task && task->Next) task = task->Next;

        CP<CUploadTask> next = new CUploadTask(sources);

        if (task) task->Next = next;
        else UploadTask = next;

        return next;
    }

    depot* CClient::GetDepot(const char* id)
    {
        CCSLock _lock(&DepotMutex, __FILE__, __LINE__);
        for (u32 i = 0; i < Depots.size(); ++i)
        {
            depot& depot = Depots[i];
            if (StringCompare(depot.Id, id) == 0)
                return &depot;
        }

        return NULL;
    }

    depot* CClient::GetDepot(u64 id)
    {
        CCSLock _lock(&DepotMutex, __FILE__, __LINE__);
        for (u32 i = 0; i < Depots.size(); ++i)
        {
            depot& depot = Depots[i];
            if (depot.DepotID == id)
                return &depot;
        }

        return NULL;
    }

    CP<CCommitTask> CClient::Commit(const CFilePath& fp, u64 depot_id)
    {
        CCSLock _commit_lock(&CommitMutex, __FILE__, __LINE__);
        CCSLock _depot_lock(&DepotMutex, __FILE__, __LINE__);

        CommitTask = new CCommitTask(fp, depot_id);

        return CommitTask;
    }

    void CClient::UpdateDownloadTasks()
    {
        CCSLock _lock(&DownloadMutex, __FILE__, __LINE__);
        for (CP<CDownloadJob>* it = DownloadsInProgress.begin(); it != DownloadsInProgress.end(); )
        {
            const CP<CDownloadJob>& job = *it;
            if (job->State == kDownloadState_Inactive)
                it = DownloadsInProgress.erase(it);
            else
                it++;
        }
    }

    void CClient::UpdateCommitTask()
    {
        CCSLock _lock(&CommitMutex, __FILE__, __LINE__);
        CP<CCommitTask>& task = CommitTask;
        if (!task) return;

        switch (task->State)
        {
            case CCommitTask::kState_Inactive:
            {
                if (!FileExists(task->DatabaseFilePath))
                {
                    SYNC_LOG("not committing changes because file database doesn't exist\n");
                    task->State = CCommitTask::kState_NoDatabaseFile;
                    break;
                }

                CCSLock _depot_lock(&DepotMutex, __FILE__, __LINE__);
                const depot* d = GetDepot(task->Depot);
                if (d == NULL)
                {
                    SYNC_LOG("not committing changes because depot wasn't found\n");
                    task->State = CCommitTask::kState_DepotNotFound;
                    break;
                }

                CFileDB database(task->DatabaseFilePath);
                if (database.Load() != REFLECT_OK)
                {
                    SYNC_LOG("failed to parse database for commit at %s\n", task->DatabaseFilePath.c_str());
                    task->State = CCommitTask::kState_DatabaseLoadFailed;
                    break;
                }

                CRawVector<CFileSource> sources(database.Files.size());
                task->CommitMessage.Files.reserve(database.Files.size());
                
                for (CFileDBRow* it = database.Files.begin(); it != database.Files.end(); ++it)
                {
                    const CFileDBRow* row = d->Database->FindByGUID(it->FileGuid);
                    
                    if (row != NULL && row->FileHash == it->FileHash)
                    {
                        SYNC_LOG("skipping upload of g%d (%s) because hash already exists in local depot cache\n", row->FileGuid.guid, row->FilePathX);
                        continue;
                    }
                    
                    u32 size;
                    CFileSource source;
                    
                    if (!it->FileHash) source = CFileSource(CFilePath(FPR_GAMEDATA, it->FilePathX));
                    else
                    {
                        for (u32 i = 0; i < CT_COUNT; ++i)
                        {
                            if (gCaches[i] != NULL && gCaches[i]->GetSize(it->FileHash, size))
                                source = CFileSource(it->FileHash);
                        }
                    }

                    if (!source.IsValid())
                    {
                        SYNC_LOG("skipping upload of g%d (%s) because data source was not found\n", it->FileGuid.guid, it->FilePathX);
                        continue;
                    }

                    if (source.IsSlow())
                        size = FileSize(source.FilePath);
                    
                    sources.push_back(source);
                    task->CommitMessage.AddFile(it->FilePathX, size, source.Hash, it->FileGuid);
                    SYNC_LOG("queueing upload of %s...\n", it->FilePathX);
                }

                SYNC_LOG("starting commit task with %d files\n", sources.size());

                task->UploadTask = Upload(sources);
                task->State = CCommitTask::kState_UploadingResources;

                break;
            }
            case CCommitTask::kState_UploadingResources:
            {
                if (task->UploadTask)
                {
                    int state = task->UploadTask->State;
                    if (state < CUploadTask::kState_Finished)
                        break;

                    if (state >= CUploadTask::kState_FinishedError)
                    {
                        task->State = CCommitTask::kState_UploadingResources;
                        break;
                    }

                }

                if (sendmsg(Socket, task->CommitMessage))
                    task->State = CCommitTask::kState_Committing;
                else
                    task->State = CCommitTask::kState_NetworkError;

                break;
            }
        }

        if (task->Completed())
            CommitTask = NULL;
    }

    void CClient::UpdateUploadTasks()
    {
        CCSLock _lock(&UploadMutex, __FILE__, __LINE__);
        CP<CUploadTask>& task = UploadTask;
        if (task)
        {
            switch (task->State)
            {
                case CUploadTask::kState_Initial:
                {
                    SYNC_LOG("Sending filter request for %d resources\n", task->Sources.size());

                    msg_resource_list msg;
                    for (CFileSource* it = task->Sources.begin(); it != task->Sources.end(); ++it)
                        msg.Hashes.push_back(it->Hash);

                    if (!sendmsg(Socket, msg))
                    {
                        task->State = CUploadTask::kState_FinishedError;
                        SYNC_LOG("Failed to send filter request for upload task, bailing!\n");
                    }
                    else 
                    {
                        task->State = CUploadTask::kState_Filtering;
                        SYNC_LOG("Waiting for filter request response...\n");
                    }
                    
                    break;
                }
                case CUploadTask::kState_Filtered:
                {
                    SYNC_LOG("Got %d (out of %d) filtered resources\n", task->Hashes.size(), task->Sources.size());

                    if (task->Hashes.size() == 0)
                    {
                        SYNC_LOG("Skipping upload task as server has all resources uploaded\n");
                        task->State = CUploadTask::kState_Finished;
                        break;
                    }

                    for (CFileSource* it = task->Sources.begin(); it != task->Sources.end(); ++it)
                    {
                        bool upload = false;
                        for (u32 i = 0; i < task->Hashes.size(); ++i)
                        {
                            if (task->Hashes[i] == it->Hash)
                                upload = true;
                        }

                        if (upload)
                        {
                            task->Jobs.push_back(CUploadJob::EnqueueForUpload(*it));
                        }
                    }

                    task->State = CUploadTask::kState_Uploading;
                    break;
                }
                case CUploadTask::kState_Uploading:
                {
                    bool finished = true;
                    bool errored = false;
                    for (u32 i = 0; i < task->Jobs.size(); ++i)
                    {
                        const CP<CUploadJob>& job = task->Jobs[i];
                        if (DownloadJobManager->CountJobsWithJobID(job->JobID))
                        {
                            finished = false;
                            break;
                        }

                        if (job->State >= kUploadState_Failed)
                            errored = true;
                    }

                    if (finished)
                    {
                        SYNC_LOG("UPLOAD TASK FINISHED : [%s]\n", errored ? "FAIL" : "OK");
                        task->State = errored ? CUploadTask::kState_FinishedError : CUploadTask::kState_Finished;
                        UploadTask = task->Next;
                    }

                    break;
                }
            }
        }    
    }

    CUploadTask::CUploadTask(const CRawVector<CFileSource>& sources) :
    State(), Sources(sources), Jobs(), Next(), Hashes()
    {
        State = kState_Initial;
    }
}