#ifndef __THREADS_H__
#define __THREADS_H__

//==================================DATA======================================//
#pragma data_seg(".data")
//============================================================================//

struct THREAD
{
	//Make thread Enlistable
	LIST_ENTRY listable;

	bool running;
	dword exit_code;
	EXECUTION state;
};

typedef dword (*fThreadFunction)(IN void* _parameters);

typedef dword HANDLE;

PRIVATE LIST_ITERATOR scheduler;
PRIVATE LIST_ENTRY threads;
PRIVATE EXECUTION main_exec;

PRIVATE bool first_time = true;
PRIVATE bool last_time = false;

//==================================CODE======================================//
#pragma code_seg(".code")
//============================================================================//

PRIVATE void ThreadStub(IN THREAD* _thread, IN fThreadFunction _code, IN void* _parameters)
{
	_thread->exit_code = _code(_parameters);
	_thread->running = false;
	for(;;);
}

PUBLIC bool ThreadInit()
{
	LIST_Init(&threads);
	return true;
}

PUBLIC bool ThreadHasFinished(IN HANDLE _thread)
{
	THREAD* t = (THREAD*)_thread;
	return !t->running;
}

PUBLIC HANDLE CreateThread(IN fThreadFunction _code, IN void* _parameters)
{
	static VIRTUAL stack_page_address_start = 0xFF000000;

	THREAD* thread = (THREAD*)HEAP_Alloc(sizeof(THREAD));
	if(!thread) return false;

	//By default thread is running and has no exit code
	thread->running = true;
	thread->exit_code = 0;

	//Alloc stack
	dword* stack = (dword*)XKY_PAGE_Alloc(XKY_ADDRESS_SPACE_GetCurrent(), stack_page_address_start, 1);
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
	stack[PAGE_SIZE/4-1] = (dword)_parameters;	//_parameters parameter
	stack[PAGE_SIZE/4-2] = (dword)_code;			//_code parameter
	stack[PAGE_SIZE/4-3] = (dword)thread;			//_thread parameter
	stack[PAGE_SIZE/4-4] = 0;						//return address

	//Fill execution
	XKY_CPU_FillExecution(&thread->state, XKY_ADDRESS_SPACE_GetCurrent(), (VIRTUAL)ThreadStub, (dword)stack + PAGE_SIZE - 4*4, true);

	//Insert in thread list
	LIST_Init((LIST_ENTRY*)thread);
	LIST_InsertTail(&threads, (LIST_ENTRY*)thread);

	//All ok
	return (HANDLE)thread;
}

//FCFS
PRIVATE bool MtCpuTimerCallbackFCFS(IN XID _xid, IN EXECUTION* _execution)
{
	if(last_time)
	{
		return false;
	}

	THREAD* thread = (THREAD*)scheduler;

	if(first_time)
	{
		first_time = false;
		main_exec = *_execution;
		*_execution = thread->state;
		return true;
	}

	if(thread->running)
	{
		thread->state = *_execution;
		return false;
	}

	while(scheduler && !thread->running)
	{
		scheduler = LIST_Next(&threads, scheduler);
		thread = (THREAD*)scheduler;
	}

	if(!scheduler)
	{
		last_time = true;
		*_execution = main_exec;
		return true;
	}

	*_execution = thread->state;
	return true; //Means another thread scheduled
}


//ROUND-ROBIN
PRIVATE LIST_ITERATOR NextThread(IN LIST_ITERATOR _iterator)
{
	_iterator = LIST_Next(&threads, _iterator);
	if(!_iterator)
		_iterator = LIST_First(&threads);
	return _iterator;
}

PRIVATE bool MtCpuTimerCallbackRoundRobin(IN XID _xid, IN EXECUTION* _execution)
{
	if(last_time)
	{
		return false;
	}

	if(first_time)
	{
		first_time = false;
		main_exec = *_execution;
		*_execution = ((THREAD*)scheduler)->state;
		return true;
	}

	//Current
	THREAD* current_thread = (THREAD*)scheduler;
	current_thread->state = *_execution;

	//Next Thread
	scheduler = NextThread(scheduler);

	if(!current_thread->running)
	{
		LIST_Remove((LIST_ENTRY*)current_thread);

		if(LIST_IsEmpty(&threads))
		{
			last_time = true;
			*_execution = main_exec;
			return true;
		}
	}

	THREAD* new_thread = (THREAD*)scheduler;

	*_execution = new_thread->state;
	return true; //Means another thread scheduled
}

#endif
