#ifndef __SPINLOCK_H__
#define __SPINLOCK_H__

//Locks
typedef volatile int LOCK;
typedef LOCK* PLOCK;

void InitLock(PLOCK lock);
void Lock(PLOCK lock);
void Unlock(PLOCK lock);

#endif
