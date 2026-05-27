#pragma once

#include <thread.h>
#include <vector.h>
#include <refcount.h>
#include <nettypes.h>
#include <Sync/Types.h>
#include <Sync/Messages.h>
#include <ResourceDescriptor.h>
#include <DebugLog.h>
#include <CritSec.h>
#include <SerialisedResource.h>
#include <JobManager.h>
#include <Sync/Shared.h>

namespace sync
{
    class CClient;
    typedef void (*MessageCallback)(CClient*, const packet& p, const ByteArray& d);
    typedef void (*StateChangeCallback)(CClient*, int state);
    typedef void (*DownloadCallback)(int, void*);

    void RegisterStateChangeCallback(StateChangeCallback cb);
    void RegisterMessageCallback(int channel, MessageCallback cb);

    enum
    {
        kDownloadState_Inactive,
        kDownloadState_InProgress,
        kDownloadState_Success,
        kDownloadState_Failed,
        kDownloadState_NoDataSource,
        kDownloadState_InvalidSession,
        kDownloadState_NetworkError,
        kDownloadState_IOError,
        kDownloadState_BadHash
    };

    enum
    {
        kUploadState_Inactive,
        kUploadState_InProgress,
        kUploadState_Success,
        kUploadState_Failed,
        kUploadState_NoDataSource,
        kUploadState_InvalidSession,
        kUploadState_NetworkError,
        kUploadState_IOError
    };

    class CDownloadJob : public CBaseCounted {
    public:
        CDownloadJob();
    public:
        inline bool IsPreload() const { return !CSR; }
    public:
        void EnqueueForDownload(const CP<CSerialisedResource>& csr, int priority);
        void Reset();
        void Finish();
    public:
        static void DownloadJob(void* userdata);
    private:
        CDownloadJob(const CDownloadJob& rhs);
        CDownloadJob& operator=(const CDownloadJob& rhs);
    public:
        CP<CSerialisedResource> CSR;
        int Priority;
        CFilePath FilePath;
        sockaddr_in Server;
        token Token;
        u32 DownloadSize;
        u32 BytesDownloaded;
        u32 JobID;
        s32 State;
    };

    class CUploadJob : public CBaseCounted {
    protected:
        CUploadJob();
    public:
        static CP<CUploadJob> EnqueueForUpload(const CFileSource& source);
    public:
        static void UploadJob(void* userdata);
    private:
        CUploadJob(const CDownloadJob& rhs);
        CUploadJob& operator=(const CDownloadJob& rhs);
    public:
        CFileSource Source;
        sockaddr_in Server;
        token Token;
        u32 UploadSize;
        u32 BytesUploaded;
        u32 JobID;
        s32 State;
    };

    class CUploadTask : public CBaseCounted {
    public:
        enum
        {
            kState_Initial,
            kState_Filtering,
            kState_Filtered,
            kState_Uploading,
            kState_Finished,
            kState_FinishedError
        };
    public:
        CUploadTask(const CRawVector<CFileSource>& sources);
    public:
        int State;
        CRawVector<CFileSource> Sources;
        CRawVector<CHash> Hashes;
        CVector<CP<CUploadJob> > Jobs;
        CP<CUploadTask> Next;
    };

    class CCommitTask : public CBaseCounted {
    public:
        enum
        {
            kState_Inactive,
            kState_UploadingResources,
            kState_Committing,
            kState_Success,
            kState_Failed,

            kState_NoDatabaseFile,
            kState_DepotNotFound,
            kState_DatabaseLoadFailed,
            kState_UploadResourcesFailed,
            kState_NetworkError
        };
    public:
        CCommitTask(const CFilePath& fp, u64 depot_id);
    public:
        inline bool InProgress() const { return State < kState_Success; }
        inline bool Succeeded() const { return State == kState_Success; }
        inline bool Failed() const { return State >= kState_Failed; }
        inline bool Completed() const { return State >= kState_Success; }
    public:
        u64 Depot;
        int State;
        CP<CUploadTask> UploadTask;
        CFilePath DatabaseFilePath;
        msg_commit CommitMessage;
    };

    class CClient {
    public:
        enum
        {
            kState_Disconnected,
            kState_Connecting,
            kState_Connected,
            kState_Error
        };

        enum
        {
            kSubState_None,
            kSubState_WaitingForServerInfo,
            kSubState_WaitingForLogin,
            kSubState_WaitingForDepotList,
            kSubState_WaitingForDepotDownloads,
            kSubState_FinishedDownloadingDepots
        };
        
        enum
        {
            kErrorState_None,
            kErrorState_Unauthorized = 1000
        };
    public:
        CClient();
        ~CClient();
    private:
        static MAKE_THREAD_FUNCTION(ThreadFunctionStatic);
        void ThreadFunction();
    public:
        inline bool IsConnecting() const { return (State == kState_Disconnected && WantConnect) || State == kState_Connecting; }
        inline bool IsConnected() const { return State == kState_Connected; }
        inline bool CanHandleCSR() const
        {
            return IsConnecting() || IsConnected();
        }
        
        inline u32 GetDownloadTag() const { return DownloadTag; }
        inline u32 GetUploadTag() const { return UploadTag; }
        inline const sockaddr_in& GetServerAddress() const { return Address; }
        inline const token& GetSession() const { return Token; }
    public:
        bool IsExtensionSupported(u32 channel, u32 protocol);
        bool Connect(const char* host, int port = kSyncServerPort);
        void Disconnect();
        void Update();
        void UpdateSynced();
    public:
        void Download(CP<CSerialisedResource>& csr, int priority);
        bool SendMessage(u32 channel, u32 message, void* data = NULL, int len = 0);
        CP<CUploadTask> Upload(const CRawVector<CFileSource>& sources);
        CP<CCommitTask> Commit(const CFilePath& fp, u64 depot_id);
    private:
        void HandleSyncChannel(const packet& packet, ByteArray& data);
        void HandleGateChannel(const packet& packet, ByteArray& data);
        void HandleResourceChannel(const packet& packet, ByteArray& data);
        void UpdateUploadTasks();
        void UpdateCommitTask();
    public:
        const depot* GetDepot(u64 id);
        const depot* GetDepot(const char* id);
    public:
        void DoUploadTest();
    public:
        int State;
        union
        {
            int SubState;
            int ErrorState;
        };
        bool WantQuit;
        bool WantConnect;

        SOCKET Socket;
        sockaddr_in Address;
        THREAD Thread;

        u64 AccountID;
        token Token;

        CP<CUploadTask> UploadTask;
        CP<CCommitTask> CommitTask;

        SServerInfo ServerInfo;
        u32 PendingDepotRequests;

        CCriticalSec DownloadMutex;
        CCriticalSec UploadMutex;
        CCriticalSec CommitMutex;

        u32 DownloadTag;
        u32 UploadTag;

        CVector<CP<CDownloadJob> > DownloadsInProgress;
    };

    extern CClient* Client;
    extern CJobManager* DownloadJobManager;
}