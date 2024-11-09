#include "GetLoginCookieTask.h"
#include <hook.h>

MH_DefineFunc(CAuthenticatedTask_PrepareTask, 0x0015a08c, TOC0, bool, CAuthenticatedTask*);
bool CAuthenticatedTask::PrepareTask()
{
    return CAuthenticatedTask_PrepareTask(this);
}