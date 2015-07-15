#ifndef __SPINLOCK_H__
#define __SPINLOCK_H__

typedef volatile dword LOCK;
typedef LOCK* PLOCK;

//==================================CODE======================================//
#pragma code_seg(".code")
//============================================================================//

//Locks
PUBLIC void InitLock(IN PLOCK _lock)
{
	*_lock = 0;
}

PUBLIC bool TryLock(IN PLOCK _lock)
{
	int lock_value;
	__asm
	{
		mov eax, _lock
		mov ebx, 1
		lock xchg ebx, [eax]
		mov lock_value, ebx
	}
	return lock_value == 0;
}

PUBLIC void AcquireLock(IN PLOCK _lock)
{
	while(*_lock || !TryLock(_lock));
}


PUBLIC void ReleaseLock(IN PLOCK _lock)
{
	*_lock = 0;
}

#endif
