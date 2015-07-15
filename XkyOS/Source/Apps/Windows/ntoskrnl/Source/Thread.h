#ifndef __THREAD_H__
#define __THREAD_H__

//==================================DATA======================================//
#pragma data_seg(".data")
//============================================================================//
struct THREAD
{
	//Make thread enlistable
	LIST_ENTRY listable;

	HANDLE process;

	dword exit_code;
	EXECUTION state;
};

PRIVATE LIST_ENTRY threads;

typedef dword (*fThreadFunction)(IN void* _parameters);

PRIVATE HANDLE idle_thread = 0;
PRIVATE bool first_time = true; 
PUBLIC LOCK threads_lock;
PRIVATE LIST_ITERATOR scheduler;

string timer_called = STRING("Windows timer called");

//==================================CODE======================================//
#pragma code_seg(".code")
//============================================================================//

PUBLIC HANDLE CreateThread(IN ADDRESS_SPACE _pdbr, IN HANDLE _process, IN fThreadFunction _code, IN void* _parameters, IN bool _user_mode)
{
	static VIRTUAL stack_page_address_start = 0xFF000000;

	THREAD* thread = (THREAD*)HEAP_Alloc(sizeof(THREAD));
	if(!thread) return false;

	//By default thread has no exit code
	thread->exit_code = 0;
	//the process the thread belongs to
	thread->process = _process;

	//Alloc stack
	dword* stack = (dword*)XKY_PAGE_Alloc(_pdbr, stack_page_address_start, 1);
	if(!stack)
	{
		HEAP_Free((VIRTUAL&)thread);
		return false;
	}
	else
	{
		stack_page_address_start += PAGE_SIZE;
	}

	//Put parameters
	if(_pdbr != XKY_ADDRESS_SPACE_GetCurrent())
	{
		XKY_PAGE_Share(_pdbr, (VIRTUAL)stack, XKY_ADDRESS_SPACE_GetCurrent(), (VIRTUAL)stack, 1);
	}
	
	stack[PAGE_SIZE/4-1] = (dword)_parameters;	//_parameters parameter
	stack[PAGE_SIZE/4-2] = 0;					//_code parameter

	if(_pdbr!=XKY_ADDRESS_SPACE_GetCurrent())
	{
		XKY_PAGE_Free(XKY_ADDRESS_SPACE_GetCurrent(), (VIRTUAL)stack, 1);
	}


	//Fill execution
	XKY_CPU_FillExecution(&thread->state, _pdbr, (VIRTUAL)_code, (VIRTUAL)stack + PAGE_SIZE - 2*4, _user_mode);

	//Insert in thread list
	LIST_Init((LIST_ENTRY*)thread);

	AcquireLock(&threads_lock);
	LIST_InsertTail(&threads, (LIST_ENTRY*)thread);
	ReleaseLock(&threads_lock);

	//All ok
	return (HANDLE)thread;
}

//Scheduler
//ROUND-ROBIN
PRIVATE LIST_ITERATOR NextThread(IN LIST_ITERATOR _iterator)
{
	_iterator = LIST_Next(&threads, _iterator);
	if(!_iterator)
		_iterator = LIST_First(&threads);
	return _iterator;
}

PUBLIC bool RoundRobinSchedulingPolicy(IN XID _xid, IN EXECUTION* _execution)
{
	if(TryLock(&threads_lock))
	{
		//Current
		THREAD* current_thread = (THREAD*)scheduler;

		if(first_time)
		{
			first_time = false;
			*_execution = current_thread->state;
			ReleaseLock(&threads_lock);
			return true;
		}

		current_thread->state = *_execution;

		//Next Thread
		scheduler = NextThread(scheduler);

		THREAD* new_thread = (THREAD*)scheduler;

		*_execution = new_thread->state;

		ReleaseLock(&threads_lock);
		return true; //Means another thread scheduled
	}
	return false;
}

PRIVATE dword IdleThread(IN void* _parameters)
{
	//Do nothing
	for(;;);
}

PUBLIC bool ThreadInit()
{
	//Init thread list
	LIST_Init(&threads);

	//Init thread lock
	InitLock(&threads_lock);

	//Create idle thread
	idle_thread = CreateThread(XKY_ADDRESS_SPACE_GetCurrent(), 0, IdleThread, 0, false);
	scheduler = LIST_First(&threads);

	//Ok
	return idle_thread != 0;
}

#endif
