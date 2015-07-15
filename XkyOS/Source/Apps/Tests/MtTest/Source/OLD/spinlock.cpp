#include "spinlock.h"

//Locks
static int TestAndSet(PLOCK the_lock)
{
	int old_value;
	__asm
	{
		mov eax, the_lock
		mov ebx, 1
		lock xchg ebx, [eax]
		mov old_value, ebx
	}
	return old_value != 0;
}

void InitLock(PLOCK lock)
{
	Unlock(lock);
}

void Lock(PLOCK lock)
{
	while(*lock || TestAndSet(lock));
}

void Unlock(PLOCK lock)
{
	*lock=0;
}
