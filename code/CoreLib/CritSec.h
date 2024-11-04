#ifndef CRITSEC_H
#define CRITSEC_H


#include "printf.h"

#include <sys/synchronization.h>
#include <sys/return_code.h>

#include "StringUtil.h"

class CCriticalSec {
public:
	sys_lwmutex_t cs; 
	const char* const Name;
	const char* LockFile;
	int LockLine;
	int DEBUGIsLocked;

	inline CCriticalSec(const char* name) : Name(name) {
		sys_lwmutex_attribute_t attr;
		attr.name[0] = '\0';
		attr.attr_protocol = SYS_SYNC_FIFO;
		attr.attr_recursive = SYS_SYNC_RECURSIVE;
		
		StringCopy<char, 8u>(attr.name, (char*) name);

		sys_lwmutex_create(&this->cs, &attr);

		this->DEBUGIsLocked = 0;
		this->LockLine = -1;
		this->LockFile = NULL;
	}
	
	inline void Enter(const char* lock_file, int lock_line)
	{ 
		bool printed = false;
		while (true) 
		{
			int res = sys_lwmutex_lock(&this->cs, 2000000);
			if (res != ETIMEDOUT)
			{
				if (res == CELL_OK) {
					this->LockFile = lock_file;
					this->LockLine = lock_line;
					this->DEBUGIsLocked++;
				}

				break;
			}

			if (!printed)
			{
				printf("delay acquiring mutex '%s' %08x retval = %08x\n", this->Name, this->cs.sleep_queue, ETIMEDOUT);
				printed = true;
			}
		}
	}

	inline void Leave()
	{
		this->DEBUGIsLocked--;
		sys_lwmutex_unlock(&this->cs);
	}
};

/* CritSec.h: 204 */
class CCSLock {
public:
	CCriticalSec* CS;

	inline CCSLock(CCriticalSec* c, const char* lock_file, int lock_line) {
		CS = c;
		if (CS != NULL)
			CS->Enter(lock_file, lock_line);
	}

	inline ~CCSLock() { if (CS != NULL) CS->Leave();  }
	inline void Set(CCriticalSec* c) { CS = c; }
};

#endif