#include <Sync/Client.h>
#include <Sync/Serialise.h>
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
// #include <network/RNPManager.h>

#include <mem_stl_buckets.h>


extern ESerialisationType GetPreferredSerialisationType(EResourceType type);
namespace sync
{
    CClient* Client;
    CJobManager* DownloadJobManager;

    const u32 kProtocolVersion = eSyncSR_LatestPlusOne - 1;
    static u32 NextMark = 1;

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
        {
            // credential based login
            return true;
        }

        return state == SCE_NP_MANAGER_STATUS_ONLINE;
    }

    bool unbuffered_send(SOCKET s, void* data, int len)
    {
        int bytes_sent = 0;
        while (bytes_sent < len)
        {
            int ret = send(s, (void*)((char*)data + bytes_sent), len - bytes_sent, 0xc00);
            if (ret <= 0) return false;
            bytes_sent += ret;
        }
        
        return true;
    }

    int recv_exactly(SOCKET s, void* buf, int len)
    {
        int n = 0;
        do
        {
            int res = recv(s, (char*)buf + n, len - n, 0);
            if (res <= 0) return res;
            n += res;
        } 
        while (n < len);

        return n;
    }

    bool recvmsg(SOCKET s, packet& p, ByteArray& data)
    {
        if (recv_exactly(s, &p, sizeof(packet)) <= 0) return false;

        u32 length = p.Length - sizeof(packet);
        data.resize(length);

        if (length > 0)
            return recv_exactly(s, data.begin(), length) > 0;
        
        return true;
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
        if (!recvmsg(s, p, data) || p.Status != kResponse_Ok || p.Message != GetMessageType<T>() || p.Channel != GetChannel<T>()) return false;
        return parsemsg(data, msg);
    }

    bool sendmsg(SOCKET s, u32 channel, u32 message, void* data, int len)
    {
        packet p;
        p.Length = sizeof(packet) + len;
        p.Message = message;
        p.Channel = channel;
        p.Method = eMethod_Request;
        p.Status = kResponse_Ok;
        p.Mark = NextMark++;
        
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

    CDownloadJob::CDownloadJob() : CBaseCounted(),
    CSR(), FilePath(), Server(), Token(), DownloadSize(),
    BytesDownloaded(), JobID(), State(kDownloadState_Inactive), Priority(STREAM_NO_STREAMING)
    {
        Reset();
    }

    void CDownloadJob::Reset()
    {
        CSR = NULL;
        FilePath.Clear();
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
        // if (!CSR)
        // {
        //     Reset();
        //     return;
        // }

        // switch (State)
        // {
        //     case kDownloadState_Success:
        //     {
        //         AddCSRToDoneQueue(CSR, Priority, 0);
        //         break;
        //     }
        //     case kDownloadState_NoDataSource:
        //     {
        //         if (!gNetworkManager.ConnectionManager.IsDownloadOK())
        //         {
        //             SetResourceError(CSR, LOAD_STATE_ERROR_NO_DATA_SOURCE);
        //             break;
        //         }

        //         // Resources that prefer to be loaded as a loose file
        //         // can't be loaded over either the HTTP or RNP server.
        //         if (GetPreferredSerialisationType(CSR->GetDescriptor().GetType()) == PREFER_FILE)
        //         {
        //             SetResourceError(CSR, LOAD_STATE_ERROR_NO_DATA_SOURCE);
        //             break;
        //         }

        //         // If we have no data source, we can now try
        //         // passing the resource to either the RNP or HTTP
        //         // thread depending on which is available.
        //         if (gRNPManager.CanHandleCSR())
        //         {
        //             AddCSRToQueue(CSR, CSRsForRNP, Priority, 0);
        //             break;
        //         }

        //         // Make sure the resource we're trying to download actually has a hash
        //         // to pass to the HTTP resource thread.
        //         CHash hash = CSR->GetDescriptor().LatestHash();
        //         if (hash)
        //         {
        //             AddCSRToQueue(CSR, CSRsForHTTP, Priority, 0);
        //             break;
        //         }

        //         SetResourceError(CSR, LOAD_STATE_ERROR_NO_DATA_SOURCE);
        //         break;
        //     }
        //     case kDownloadState_InvalidSession:
        //     case kDownloadState_BadHash:
        //     {
        //         // requeue the download for bad hash
        //         // request a new session for invalid session
        //         SetResourceError(CSR, LOAD_STATE_ERROR_NO_DATA_SOURCE);
        //         break;
        //     }
        //     default:
        //     {
        //         SetResourceError(CSR, LOAD_STATE_ERROR_NO_DATA_SOURCE);
        //         break;
        //     }
        // }

        // Reset();
    }

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

        bool StreamToFile(const CFilePath& fp)
        {
            FileHandle fd;
            
            DirectoryCreate(fp);
            if (!FileOpen(fp, fd, OPEN_WRITE))
                return false;

            const int kBufferSize = 4096;
            CSHA1Context sha1;
            char buf[kBufferSize];

            int n = 0;
            while (n < Response.FileSize)
            {
                ssize_t rv = recv_exactly(Socket, buf, MIN(Response.FileSize - n, kBufferSize));
                if (rv <= 0)
                {
                    FileClose(fd);
                    FileUnlink(fp);
                    return false;
                }

                sha1.AddData((const u8*)buf, rv);
                FileWrite(fd, (void*)buf, rv);
                n += rv;
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
            CFartRO* cache = (CFartRO*)gCaches[CT_SYNC];
            if (cache == NULL) return false;
            CCSLock lock(&cache->Mutex, __FILE__, __LINE__);

            CFartRO::CFAT fat;
            fat.hash = Response.FileHash;
            fat.size = Response.FileSize;
            fat.offset = 0;

            for (CFartRO::CFAT* it = cache->FAT.begin(); it != cache->FAT.end(); ++it)
            {
                fat.offset = MAX(fat.offset, it->offset + it->size);

                // Hash is already in the FAT, we can just ignore this data.
                if (it->hash == fat.hash) return true;
            }

            FileHandle fd;
            if (!FileOpen(cache->fp, fd, OPEN_RDWR)) 
                return false;
            
            cache->FAT.push_back(fat);
            std::sort(cache->FAT.begin(), cache->FAT.end(), std::less<CFartRO::CFAT>());

            Footer footer;
            footer.count = cache->FAT.size();
            footer.magic = FARC;

            FileSeek(fd, fat.offset, FILE_BEGIN);
            FileWrite(fd, (void*)data.begin(), data.size());
            FileWrite(fd, (void*)cache->FAT.begin(), cache->FAT.size() * sizeof(CFartRO::CFAT));
            FileWrite(fd, (void*)&footer, sizeof(Footer));
            FileClose(fd);

            return true;
        }

        bool Work()
        {
            CResourceDescriptorBase desc = Owner->CSR->GetDescriptor();
            msg_download rqst;
            rqst.GUID = desc.GetGUID();
            rqst.Hash = desc.GetHash();
            rqst.Token = Owner->Token;

            if (!sendmsg(Socket, rqst))
                return false;
            
            if (!recvmsg(Socket, Response))
                return false;
            
            SYNC_LOG("download worker got fileinfo size=%d, hash=%s, is_compressed=%s", Response.FileSize, StringifyHash(Response.FileHash).c_str(), Response.IsCompressed ? "true" : "false");

            if (Response.IsCompressed)
            {
                SYNC_LOG("compressed data isn't supported in download responses!\n");
                return false;
            }
            else
            {
                if (GetPreferredSerialisationType(desc.GetType()) == PREFER_FILE)
                {
                    CFilePath fp;
                    StreamToFile(fp);
                }
                else
                {
                    ByteArray data;
                    data.resize(Response.FileSize);
                    if (recv_exactly(Socket, data.begin(), data.size()) <= 0)
                    {
                        SYNC_LOG("an error occurred in worker while receiving file data!\n");
                        return false;
                    }

                    if (!StreamToCache(data))
                        return false;

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
            if (d->State < kDownloadState_Failed)
                d->State = kDownloadState_NoDataSource;
            SYNC_LOG("DOWNLOAD FAILED!!!\n");
        }
        else
        {
            d->State = kDownloadState_Success;
            SYNC_LOG("DOWNLOAD SUCCESS!!!\n");
        }

        d->Finish();
    }

    void DownloadTest(void* userdata)
    {
        SYNC_LOG("download test, making sure tag is correct!!!: %08x\n", ((CClient*)userdata)->DownloadTag);
    }

    CClient::CClient() : WantQuit(false), WantConnect(true),
    State(kState_Disconnected), SubState(kSubState_None), Socket(-1), Address(), Thread(),
    DepotMutex("DepotMutex"), Depots(),
    DownloadMutex("DownloadMutex"), UploadMutex("UploadMutex"), DownloadTag(), DownloadsInProgress(),
    ServerInfo(), UploadTask(), PendingDepotRequests()
    {
        // Using the HTTP job manager because it's probably going to be
        // less congested than the main job manager.
        DownloadTag = DownloadJobManager->GetUniqueTag();
        UploadTag = DownloadJobManager->GetUniqueTag();
        SYNC_LOG("constructed sync client, download tag = 0x%08x, test download job = 0x%08x, sizeof(packet)=0x%08x\n", DownloadTag,
            DownloadJobManager->EnqueueJob(1000, &DownloadTest, (void*)this, DownloadTag, "Download Test"), sizeof(packet)
        );

        LoadDepotCache();
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
                    client->Connect("localhost", 16723);
                    client->WantConnect = false;
                }
            }
            else client->Update();

            ThreadSleep(33);
        }
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

        int opt = 1;
        if (setsockopt(Socket, SOL_SOCKET, SO_NBIO, &opt, sizeof(int)) != 0)
        {
            SYNC_LOG("failed to set socket to non blocking!\n");
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
        
        CP<CDownloadJob> dl = new CDownloadJob();
        CCSLock lock(&DownloadMutex, __FILE__, __LINE__);
        DownloadsInProgress.push_back(dl);
        dl->EnqueueForDownload(csr, priority);
    }

    bool CClient::SendMessage(u32 channel, u32 message, void* data, int size)
    {
        if (!sendmsg(Socket, channel, message, data, size))
        {
            SYNC_LOG("failed to send message to server!\n");
            Disconnect();
            return false;
        }

        return true;
    }
    
    void CClient::HandleSyncChannel(const packet& packet, const ByteArray& data)
    {
        switch (packet.Message)
        {

        }
    }

    void CClient::HandleGateChannel(const packet& packet, const ByteArray& data)
    {
        switch (packet.Message)
        {

        }
    }

    void CClient::UpdateSynced()
    {
        // CP<RFileOfBytes> file = LoadResourceByKey<RFileOfBytes>(1);
        // if (file->IsLoaded())
        //     SYNC_LOG("%s\n", (const char*)file->Data.begin());
    }

    void CClient::Update()
    {
        if (State == kState_Disconnected || State == kState_Error) return;
        
        packet p;
        ByteArray data;
        if (recvmsg(Socket, p, data))
        {
            if (p.Method != eMethod_Response && p.Message != eMethod_Event)
            {
                SYNC_LOG("received packet with invalid method from server!\n");
                Disconnect();
                return;
            }

            if (State == kState_Connected)
            {
                switch (p.Channel)
                {
                    case eChannel_Gate: HandleGateChannel(p, data); break;
                    case eChannel_Sync: HandleSyncChannel(p, data); break;
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
                            SYNC_LOG("expected server info on client wakeup!\n");
                            Disconnect();
                            return;
                        }

                        if (!parsemsg(data, ServerInfo))
                        {
                            SYNC_LOG("failed to parse server info reply from the server!\n");
                            Disconnect();
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
                            SYNC_LOG("protocol version mismatch: %d != %d\n", kProtocolVersion, ServerInfo.ProtocolVersion);
                            Disconnect();
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
                                SYNC_LOG("failed to get np ticket, defaulting to dummy r/o creds!\n");
                                login.Method = eAuthenticationType_Credentials;
                                login.Username = "mold";
                                login.Password = "dummy123";
                            }
                        }

                        if (!sendmsg(Socket, login))
                        {
                            SYNC_LOG("failed to send login message to server!\n");
                            Disconnect();
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
                            SYNC_LOG("expected login response from server!\n");
                            Disconnect();
                            return;
                        }

                        if (p.Status != kResponse_Ok)
                        {
                            SYNC_LOG("failed to login!\n");
                            Disconnect();
                            return;
                        }

                        msg_login_response resp;
                        if (!parsemsg(data, resp))
                        {
                            SYNC_LOG("failed to parse login reply from server!\n");
                            Disconnect();
                            return;
                        }

                        SYNC_LOG("successfully logged into alear server\n");

                        AccountID = resp.AccountID;
                        Token = resp.Token;

                        SYNC_LOG("token bytes: %08x\n", *(uint32_t*)&resp.Token);

                        if (!sendmsg(Socket, eChannel_Sync, eMessageType_DepotList))
                        {
                            SYNC_LOG("failed to send request for depot list!\n");
                            Disconnect();
                            return;
                        }

                        SubState = kSubState_WaitingForDepotList;

                        break;
                    }
                    case kSubState_WaitingForDepotList:
                    {
                        if (p.Message != eMessageType_DepotList)
                        {
                            SYNC_LOG("expected depot list response from server!\n");
                            Disconnect();
                            return;
                        }

                        if (p.Status != kResponse_Ok)
                        {
                            SYNC_LOG("failed to fetch depot list!\n");
                            Disconnect();
                            return;
                        }

                        CVector<depot> server_depots;
                        CVector<depot>& depot_cache = Depots;

                        if (!parsemsg(data, server_depots))
                        {
                            SYNC_LOG("failed to parse depot list reply from server!\n");
                            Disconnect();
                            return;
                        }

                        PendingDepotRequests = 0;

                        SYNC_LOG("Depots:\n");
                        for (u32 i = 0; i < server_depots.size(); ++i)
                        {
                            const depot& d = server_depots[i];

                            const depot* c = GetDepot(d.DepotID);
                            if (c == NULL || c->CommitID != d.CommitID)
                            {
                                sendmsg(Socket, eChannel_Sync, eMessageType_Depot, (void*)&d.DepotID, sizeof(u64));
                                PendingDepotRequests++;
                            }

                            SYNC_LOG("[%s] %s, prio = %d\n", d.Id.c_str(), d.Name.c_str(), d.Priority);
                        }

                        server_depots.swap(depot_cache);
                        SaveToDepotCache();
                        SubState = kSubState_WaitingForDepotDownloads;

                        break;
                    }
                    case kSubState_WaitingForDepotDownloads:
                    {
                        if (p.Message != eMessageType_Depot)
                        {
                            SYNC_LOG("expected depot response from server!\n");
                            Disconnect();
                            return;
                        }

                        if (p.Status != kResponse_Ok)
                        {
                            SYNC_LOG("failed to fetch depot!\n");
                            Disconnect();
                            return;
                        }

                        msg_depot_response resp;
                        if (!parsemsg(data, resp))
                        {
                            SYNC_LOG("failed to parse depot reply from server!\n");
                            Disconnect();
                            return;
                        }

                        const depot* depot = GetDepot(resp.DepotID);

                        CFilePath fp(FPR_GAMEDATA, "gamedata/alear/sync/depots");
                        fp.Append(depot->Id.c_str());
                        fp.AppendRaw(".map");

                        int fd;
                        if (FileOpen(fp, fd, OPEN_WRITE))
                        {
                            FileWrite(fd, resp.FileData.begin(), resp.FileData.size());
                            FileClose(fd);
                        }

                        PendingDepotRequests--;
                        if (PendingDepotRequests != 0) break;

                        State = kState_Connected;
                        SubState = kSubState_None;

                        for (StateChangeCallback* cb = RegisteredStateCallbacks.begin(); cb != RegisteredStateCallbacks.end(); ++cb)
                            (*cb)(this, kState_Connected);

                        break;
                    }
                }
            }
        }
        else if (net_errno != EWOULDBLOCK)
        {
            SYNC_LOG("failed to recv msg from server, disconnected\n");
            Disconnect();
            return;
        }

        if (State == kState_Connecting && SubState == kSubState_None)
        {
            SYNC_LOG("requesting server information\n");
            sendmsg(Socket, eChannel_Gate, eMessageType_ServerInfo);
            SubState = kSubState_WaitingForServerInfo;
        }
    }

    void CClient::DoUploadTest()
    {
        SYNC_LOG("doing upload test\n");
        CRawVector<CFileSource> sources;
        sources.push_back(CFilePath(FPR_GAMEDATA, "cwml/data/test.bin"));
        Upload(sources);
    }

    void CClient::Upload(const CRawVector<CFileSource>& sources)
    {
        if (sources.size() == 0) return;

        CP<CUploadTask> task = new CUploadTask(sources);
        if (UploadTask != NULL)
            task->Next = UploadTask;
        UploadTask = task;
    }

    const depot* CClient::GetDepot(u64 id)
    {
        CCSLock _lock(&DepotMutex, __FILE__, __LINE__);
        for (u32 i = 0; i < Depots.size(); ++i)
        {
            const depot& depot = Depots[i];
            if (depot.DepotID == id)
                return &depot;
        }

        return NULL;
    }

    CFilePath CClient::GetDepotCacheFilePath() const
    {
        return CFilePath(FPR_GAMEDATA, "gamedata/alear/sync/depotcache");
    }

    void CClient::DestroyDepotCache()
    {
        FileUnlink(GetDepotCacheFilePath());
    }

    void CClient::LoadDepotCache()
    {
        CCSLock _lock(&DepotMutex, __FILE__, __LINE__);
        Depots.resize(0);

        ByteArray b;
        if (!FileLoad(GetDepotCacheFilePath(), b)) return;

        CReflectionLoadVector r(&b);
        r.SetCompressionFlags(0);
        u32 version;
        if (Reflect(r, version) != REFLECT_OK || version > kProtocolVersion)
        {
            DestroyDepotCache();
            return;
        }

        r.SetRevision(SRevision(version));
        if (Reflect(r, Depots) != REFLECT_OK)
        {
            Depots.resize(0);
            DestroyDepotCache();
        }
    }

    void CClient::SaveToDepotCache()
    {
        CCSLock _lock(&DepotMutex, __FILE__, __LINE__);

        CSyncSaveVector r;
        u32 version = kProtocolVersion;
        if (Reflect(r, version) != REFLECT_OK) return;
        if (Reflect(r, Depots) != REFLECT_OK) return;

        int fd;
        if (FileOpen(GetDepotCacheFilePath(), fd, OPEN_WRITE))
        {
            FileWrite(fd, r.Data.begin(), r.Data.size());
            FileClose(fd);
        }
    }

    void CClient::UpdateUploadTasks()
    {
        CP<CUploadTask>& task = UploadTask;
        while (task)
        {
            switch (task->State)
            {
                case CUploadTask::kState_Initial:
                {
                    task->FilterRequest = NextMark;

                    msg_resource_list msg;
                    for (CFileSource* it = task->Sources.begin(); it != task->Sources.end(); ++it)
                        msg.Hashes.push_back(it->Hash);

                    if (!sendmsg(Socket, msg))
                        task->State = CUploadTask::kState_FinishedError;
                    else 
                        task->State = CUploadTask::kState_Filtering;
                    
                    break;
                }
                case CUploadTask::kState_Filtered:
                {
                    break;
                }
            }

            task = task->Next;
        }    
    }

    CUploadTask::CUploadTask(const CRawVector<CFileSource>& sources) :
    State(), Sources(sources), Jobs(), Next(), FilterRequest()
    {
        State = kState_Initial;
    }
}