#include "HTTPClient.h"
#include "CritSec.h"
#include "StringUtil.h"
#include "cell/DebugLog.h"
#include "sha1.h"
#include <hook.h>

MH_DefineFunc(ParseURL, 0x0057de50, TOC1, void, char const*, CHTTPURL&);
MH_DefineFunc(CHTTPMessage_AddHeader, 0x0057f1f8, TOC1, void, CHTTPMessage*, char const*, char const*, EHTTPHeaderMode);
MH_DefineFunc(CHTTPTaskBase_Destroy, 0x00587cb4, TOC1, void, CHTTPTaskBase*);
MH_DefineFunc(CHTTPAsyncTask_StartTask, 0x0057e2e8, TOC1, void, CHTTPAsyncTask*, CHTTPResponseBase*, HTTPQuitFunc);
MH_DefineFunc(CHTTPAsyncTask_AbortTaskAndWait, 0x0057d830, TOC1, void, CHTTPAsyncTask*);
MH_DefineFunc(CHTTPAsyncTask_RestartTask, 0x0057e224, TOC1, void, CHTTPAsyncTask*);
MH_DefineFunc(CHTTPAsyncTask_StopTask, 0x0057e938, TOC1, void, CHTTPAsyncTask*);

void CHTTPMessage::AddHeader(char const* header, char const* value, EHTTPHeaderMode mode)
{
    CHTTPMessage_AddHeader(this, header, value, mode);
}

void CHTTPRequest::InitGET(char const* url, bool accept_deflate)
{
    ParseURL(url, URL);
    AddHeader("HOST", URL.host.c_str(), HTTP_HEADER_REPLACE);
    if (accept_deflate)
        AddHeader("accept-encoding", "deflate", HTTP_HEADER_REPLACE);
    Body = NULL;
    Method = "GET";
}

void CHTTPRequest::InitPOST(char const* url, ByteArray const* req_ptr, char const* type, bool accept_deflate)
{
    ParseURL(url, URL);
    AddHeader("HOST", URL.host.c_str(), HTTP_HEADER_REPLACE);

    Method = "POST";
    int len = 0;
    if (req_ptr == NULL || req_ptr->size() == 0) Body = NULL;
    else
    {
        Body = req_ptr;
        len = req_ptr->size();
    }

    char buf[64];
    FormatString<64>(buf, "%d", len);

    AddHeader("Content-Type", type, HTTP_HEADER_REPLACE);
    AddHeader("Content-Length", buf, HTTP_HEADER_REPLACE);
    if (accept_deflate)
        AddHeader("accept-encoding", "deflate", HTTP_HEADER_REPLACE);
}

bool CHTTPResponseBase::Reserve(u32 size) { return true; }
bool CHTTPResponseBase::Append(const u8* data, u32 size)
{
    TotalSize += size;
    Sha1.AddData(data, size);
    return true;
}

void CHTTPResponseBase::Reset()
{
    Sha1.Reset();
    ContentDigest.Clear();
    TotalSize = 0;
}

void CHTTPResponseBase::PrintToTTY() {}

void CHTTPResponseBase::Finish(bool zipped, unsigned int unzipped_length, char const* cookie_for_digest, char const* url_for_digest, int& status_code)
{
    static const char* xdigs = "CustomServerDigest";
    // static const char* xdigs = "!?/*hjk7duOZ1f@daX";
    CSHA1Context digest;
    memcpy(&digest, &Sha1, sizeof(CSHA1Context));
    if (Hash != NULL) Sha1.Result((uint8_t*)Hash);
    if (cookie_for_digest != NULL)
    {
        digest.AddData((uint8_t*)cookie_for_digest, StringLength(cookie_for_digest));
        digest.AddData((uint8_t*)url_for_digest, StringLength(url_for_digest));
        digest.AddData((uint8_t*)xdigs, StringLength(xdigs));
        digest.Result((uint8_t*)&ContentDigest);
    }
}

void CHTTPResponseArray::Reset()
{
    CHTTPResponseBase::Reset();
    if (Array != NULL) Array->try_resize(0);
}

bool CHTTPResponseArray::Reserve(u32 size)
{
    if (Array != NULL) return Array->try_reserve(size);
    return true;
}

bool CHTTPResponseArray::Append(const u8* data, u32 size)
{
    if (!CHTTPResponseBase::Append(data, size)) return false;
    if (Array == NULL) return true;

    int len = Array->size();
    if (!Array->try_resize(len + size)) return false;
    memcpy(Array->begin() + len, data, size);
    return true;
}

void CHTTPResponseArray::PrintToTTY()
{
    if (Array != NULL && Array->size() != 0)
        DebugLogChR(DC_HTTP, Array->begin(), Array->end());
}

void CHTTPResponseArray::Finish(bool zipped, unsigned int unzipped_length, char const* cookie_for_digest, char const* url_for_digest, int& status_code)
{
    CHTTPResponseBase::Finish(zipped, unzipped_length, cookie_for_digest, url_for_digest, status_code);
    // not supporting zipped data right now
    if (zipped)
        status_code = CELL_HTTP_STATUS_CODE_Bad_Request;
}

void CHTTPTaskBase::Destroy() { CHTTPTaskBase_Destroy(this); }
void CHTTPTaskBase::Abort()
{
    //CCSLock _the_lock(NULL, __FILE__, __LINE__);
    if (TransID != NULL)
        cellHttpTransactionAbortConnection(TransID);
}

void CHTTPTaskBase::MakeHTTPRequest(char const* url, ByteArray const*  req_ptr, EPostOrGet post_or_get, EHttpQueuePriority priority, char const* type, EHttpContentDigest digest_mode, bool accept_deflate)
{
    Request.ClearHeaders();
    DigestMode = digest_mode;
    Priority = priority;

    if (post_or_get == E_HTTP_POST) Request.InitPOST(url, req_ptr, type, accept_deflate);
    else if (post_or_get == E_HTTP_GET) Request.InitGET(url, accept_deflate);
}

void CHTTPTaskBase::AbortTaskForShutdown()
{
    if (Status != E_HTTP_TASK_INIT) 
        Abort();
}

CHTTPTaskBase::~CHTTPTaskBase() {};
bool CHTTPTaskBase::PrepareTask() { return true; }

void CHTTPTaskBase::FinaliseTask(u32 bandwidth_up, u32 bandwidth_down)
{
    if (Code != CELL_HTTP_STATUS_CODE_Internal_Server_Error)
    {
        Destroy();
        return;
    }

    DebugLogChF(DC_HTTP, "SERVER INTERNAL ERROR\n");
    DebugLogChF(DC_HTTP, "Response:\n");
    if (Response != NULL) Response->PrintToTTY();
    DebugLogChF(DC_HTTP, "\nEnd Response\n");
    Destroy();
}

bool CHTTPTaskBase::IsAborted() 
{
    if (QuitFunc != NULL)
        return QuitFunc();
    return false;
}

CHTTPAsyncTask::~CHTTPAsyncTask()
{
    StopTask();
}

bool CHTTPAsyncTask::IsAborted()
{
    return Quit || CHTTPTaskBase::IsAborted();
}

void CHTTPAsyncTask::AbortTaskAndWait()
{
    CHTTPAsyncTask_AbortTaskAndWait(this);
}

void CHTTPAsyncTask::RestartTask()
{
    CHTTPAsyncTask_RestartTask(this);
}

void CHTTPAsyncTask::StartTask(CHTTPResponseBase* response, HTTPQuitFunc quit_fn)
{
    CHTTPAsyncTask_StartTask(this, response, quit_fn);
}

void CHTTPAsyncTask::StopTask()
{
    CHTTPAsyncTask_StopTask(this);
}
