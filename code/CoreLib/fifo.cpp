#include <sys/return_code.h>

#include "fifo.h"

/* fifo.cpp: 109 */
CMMSemaphore::CMMSemaphore(int icount, int imaxcount) 
: Abort(false), Sem(0) 
{
	sys_semaphore_attribute_t sem_attr;
	sys_semaphore_create(
		&this->Sem, 
		&sem_attr,
		icount,
		imaxcount > 0 ? imaxcount : 0x7fffffff
	);
}

/* fifo.cpp: 121 */
CMMSemaphore::~CMMSemaphore() {
	if (this->Sem != 0) {
		sys_semaphore_destroy(this->Sem);
		this->Sem = 0;
	}
}

/* fifo.cpp: 130 */
bool CMMSemaphore::Increment(u32 timeout) {
	if (!this->Abort && sys_semaphore_post(this->Sem, timeout) == CELL_OK)
		return !this->Abort;
	return false;
}

/* fifo.cpp: 138 */
bool CMMSemaphore::WaitAndDecrement(int timeout) {
	int ret;
	if (timeout == 0) ret = sys_semaphore_trywait(this->Sem);
	else ret = sys_semaphore_wait(this->Sem, timeout != -1 ? timeout * 1000 : 0);
	if (ret != CELL_OK) return false;
	return !this->Abort;
}

/* MATCHES: fifo.cpp: 146 */
void CMMSemaphore::DoAbort() {
	if (this->Abort == false) {
		this->Abort = true;
		sys_semaphore_post(this->Sem, 1000);
	}
}

bool CMMSemaphore::Aborted() { return this->Abort; }