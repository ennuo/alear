#include "GetLoginCookieTask.h"
#include <hook.h>

MH_DefineFunc(CAuthenticatedTask_PrepareTask, 0x0015a08c, TOC0, bool, CAuthenticatedTask*);
bool CAuthenticatedTask::PrepareTask()
{
    return CAuthenticatedTask_PrepareTask(this);
}

MH_DefineFunc(GetNPTicket, 0x001596b4, TOC0, bool);