#ifndef FIFO_H
#define FIFO_H

#include <sys/synchronization.h>
#include "vector.h"

class CMMSemaphore {
public:
	CMMSemaphore(int initial_val, int max_val);
	~CMMSemaphore();
	// bool Increment();
	bool Increment(u32 timeout);
	bool WaitAndDecrement(int timeout);
	void DoAbort();
	bool Aborted();
private:
	sys_semaphore_t Sem;
	bool Abort;
};

#endif // FIFO_H