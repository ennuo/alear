#ifndef GET_LOGIN_COOKIE_TASK_H
#define GET_LOGIN_COOKIE_TASK_H

#include <HTTPClient.h>

enum EAuthenticatedTaskUpdateResult {
    ATUR_NOT_RUNNING,
    ATUR_STILL_RUNNING,
    ATUR_FINISHED_SUCCESS,
    ATUR_FINISHED_FAILURE
};

class CAuthenticatedTask : public CHTTPAsyncTask {
protected:
    bool PrepareTask();
private:
    u32 CookieID;
    u32 RetryAttempts;
};

#endif // GET_LOGIN_COOKIE_TASK_H