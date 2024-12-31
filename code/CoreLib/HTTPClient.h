#ifndef HTTP_CLIENT_H
#define HTTP_CLIENT_H

#include "MMString.h"
#include "GuidHash.h"
#include "sha1.h"
#include "mem_stl_buckets.h"
#include "vector.h"

#include <cell/http.h>
#include <map>

typedef bool (*HTTPQuitFunc)(void);
typedef std::map<MMString<char>, MMString<char>, std::less<MMString<char> >, STLBucketAlloc<MMString<char> > > HeaderMap;

enum EURLScheme {
    URL_UNKNOWN,
    URL_HTTP,
    URL_HTTPS
};

enum EHttpContentDigest {
    E_HTTP_DIGEST_NONE=0,
    E_HTTP_DIGEST_REQUEST_FULL=1,
    E_HTTP_DIGEST_REQUEST_SLIM=2,
    E_HTTP_DIGEST_RESPONSE_FULL=4,
    E_HTTP_DIGEST_BOTH_FULL=5,
    E_HTTP_DIGEST_RESPONSE_SLIM=8
};

enum EHTTPHeaderMode {
    HTTP_HEADER_REPLACE=0,
    HTTP_HEADER_APPEND_COMMA=1,
    HTTP_HEADER_APPEND_SEMICOLON=2,
    HTTP_HEADER_IGNORE=3
};

enum EHttpQueuePriority {
    E_HTTP_PRIORITY_LOW=0,
    E_HTTP_PRIORITY_MEDIUM=1,
    E_HTTP_PRIORITY_HIGH=2,
    E_HTTP_PRIORITY_START_STRAIGHTAWAY=3,
    E_HTTP_PRIORITY_COUNT=4
};

enum EHTTPTaskStatus {
    E_HTTP_TASK_INIT=0,
    E_HTTP_TASK_STARTED=1,
    E_HTTP_TASK_REQUESTING=2,
    E_HTTP_TASK_REQUESTED=3,
    E_HTTP_TASK_RESPONDING=4,
    E_HTTP_TASK_FAIL=5,
    E_HTTP_TASK_DONE=6
};

enum EPostOrGet {
    E_HTTP_POST=0,
    E_HTTP_GET=1
};

class CHTTPURL {
public:
    inline CHTTPURL() : scheme(), host(), port(), path(), query(), anchor() {}
public:
    EURLScheme scheme;
    MMString<char> host;
    int port;
    MMString<char> path;
    MMString<char> query;
    MMString<char> anchor;
};

class CHTTPMessage { // file.h: 109
public:
    inline CHTTPMessage() : Headers(), Body() {}
public:
    void AddHeader(char const* header, char const* value, EHTTPHeaderMode mode);
    inline void ClearHeaders() { Headers.clear(); }
protected:
    HeaderMap Headers;
    const ByteArray* Body;
};

class CHTTPRequest : public CHTTPMessage { // file.h: 148
public:
    inline CHTTPRequest() : CHTTPMessage(), Method(), URL(), CookieForDigest() {}
public:
    void InitGET(char const* url, bool accept_deflate);
    void InitPOST(char const* url, ByteArray const* req_ptr, char const* type, bool accept_deflate);
protected:
    const char* Method;
    CHTTPURL URL;
    MMString<char> CookieForDigest;
};

class CHTTPResponseBase {
protected:
    inline CHTTPResponseBase() : Sha1(), ContentDigest(), Hash(), TotalSize() {}
public:
    inline virtual ~CHTTPResponseBase() {};
    virtual bool Reserve(u32 size);
    virtual bool Append(const u8* data, u32 size);
    virtual void Finish(bool zipped, unsigned int unzipped_length, char const* cookie_for_digest, char const* url_for_digest, int& status_code);
    virtual void Reset();
    virtual void PrintToTTY();
public:
    inline u32 GetTotalSize() { return TotalSize; }
    const CHash& GetContentDigest() { return ContentDigest; }
protected:
    CSHA1Context Sha1;
    CHash ContentDigest;
    CHash* Hash;
    u32 TotalSize;
};

class CHTTPResponseArray : public CHTTPResponseBase { // file.h: 202
public:
    bool Reserve(u32 size);
    bool Append(const u8* data, u32 size);
    void Finish(bool zipped, unsigned int unzipped_length, char const* cookie_for_digest, char const* url_for_digest, int& status_code);
    void Reset();
    void PrintToTTY();
protected:
    ByteArray* Array;
};

class CHTTPTaskBase { // file.h: 318
protected:
    inline CHTTPTaskBase() : Request(), Response(), QuitFunc(),
    Status(E_HTTP_TASK_INIT), Code(CELL_HTTP_STATUS_CODE_Bad_Request), Errno(0),
    Priority(E_HTTP_PRIORITY_LOW), TransID(), DigestMode(E_HTTP_DIGEST_BOTH_FULL)
    {
    }

    virtual ~CHTTPTaskBase();
    virtual bool PrepareTask();
    virtual void FinaliseTask(u32 bandwidth_up, u32 bandwidth_down);
    virtual bool IsAborted();

    void AbortTaskForShutdown();
    void Destroy();
    void Abort();
public:
    void MakeHTTPRequest(char const* url, ByteArray const* req_ptr, EPostOrGet post_or_get, EHttpQueuePriority priority, char const* type, EHttpContentDigest digest_mode, bool accept_deflate);
public:
    inline int GettErrno() { return Errno; }
    inline EHTTPTaskStatus GetTaskStatus() { return Status; }
    inline int GetCode() { return Code; }
protected:
    // Request starts @ 0x10 in debug, 0x8 in deploy, added alignment, or more fields?
    // might just be alignment?
    char Pad[0xc];
    CHTTPRequest Request;
    CHTTPResponseBase* Response;
    HTTPQuitFunc QuitFunc;
    volatile EHTTPTaskStatus Status;
    volatile int Code;
    volatile int Errno;
    EHttpQueuePriority Priority;
    CellHttpTransId TransID;
    EHttpContentDigest DigestMode;
};

class CHTTPAsyncTask : public CHTTPTaskBase { // file.h: 390
public:
    ~CHTTPAsyncTask();
    inline CHTTPAsyncTask() : CHTTPTaskBase(), JobID(0), Quit(0) {}
public:
    bool IsAborted();
    void AbortTaskAndWait();
    void RestartTask();
    void StartTask(CHTTPResponseBase* response, HTTPQuitFunc quit_fn);
    virtual void StopTask();
protected:
    u32 JobID;
    bool Quit;
};


#endif // HTTP_CLIENT_H