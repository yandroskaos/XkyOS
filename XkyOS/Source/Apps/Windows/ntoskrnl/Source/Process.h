#ifndef __PROCESS_H__
#define __PROCESS_H__

//==================================DATA======================================//
#pragma data_seg(".data")
//============================================================================//

struct PROCESS
{
	//Make process enlistable
	LIST_ENTRY listable;

	HANDLE main_thread;
	ADDRESS_SPACE pdbr;
};

PRIVATE LIST_ENTRY processes;

PRIVATE string ntdll_name = STRING("WINDOWS\\ntdll.dll");

#define NTDLL_BASE_ADDRESS		0x7FF00000
#define EXECUTABLE_BASE_ADDRESS	0x10000000

//==================================CODE======================================//
#pragma code_seg(".code")
//============================================================================//

PUBLIC HANDLE CreateProcess(IN string* _module)
{
	//Allocate process
	PROCESS* process = (PROCESS*)HEAP_Alloc(sizeof(PROCESS));
	if(!process) return 0;

	//Create address space
	process->pdbr = XKY_ADDRESS_SPACE_Alloc();
	if(!process->pdbr)
	{
		HEAP_Free((VIRTUAL&)process);
		return 0;
	}

	//Map ntdll
	if(!XKY_LDR_LoadFile(&ntdll_name, process->pdbr, NTDLL_BASE_ADDRESS))
	{
		XKY_ADDRESS_SPACE_Free(process->pdbr);
		HEAP_Free((VIRTUAL&)process);
		return 0;
	}

	//Map _module
	if(!XKY_LDR_LoadFile(_module, process->pdbr, EXECUTABLE_BASE_ADDRESS))
	{
		XKY_ADDRESS_SPACE_Free(process->pdbr);
		HEAP_Free((VIRTUAL&)process);
		return 0;
	}

	//Resolve imports in newly created address space
		//Share both images
	dword pages_for_ntdll = RTL_BytesToPages(XKY_LDR_FileSize(&ntdll_name));
	if(!XKY_PAGE_Share(process->pdbr, NTDLL_BASE_ADDRESS, XKY_ADDRESS_SPACE_GetCurrent(), NTDLL_BASE_ADDRESS, pages_for_ntdll))
	{
		XKY_ADDRESS_SPACE_Free(process->pdbr);
		HEAP_Free((VIRTUAL&)process);
		return 0;
	}

	dword pages_for_executable = RTL_BytesToPages(XKY_LDR_FileSize(_module));
	if(!XKY_PAGE_Share(process->pdbr, EXECUTABLE_BASE_ADDRESS, XKY_ADDRESS_SPACE_GetCurrent(), EXECUTABLE_BASE_ADDRESS, pages_for_executable))
	{
		XKY_PAGE_Free(XKY_ADDRESS_SPACE_GetCurrent(), NTDLL_BASE_ADDRESS, pages_for_ntdll);
		XKY_ADDRESS_SPACE_Free(process->pdbr);
		HEAP_Free((VIRTUAL&)process);
		return 0;
	}

	//Here we should do things the right way, but for speed we're hardcoding
	//Ok, now resolve exports/imports
	*((dword*)0x10002008) = 0x7FF01000;	//MessageBox
	*((dword*)0x10002000) = 0x7FF01030;	//ExitProcess

	//Get entry point
	dword entry_point = 0x10001000;

	//Unmap pages
	XKY_PAGE_Free(XKY_ADDRESS_SPACE_GetCurrent(), NTDLL_BASE_ADDRESS, pages_for_ntdll);
	XKY_PAGE_Free(XKY_ADDRESS_SPACE_GetCurrent(), EXECUTABLE_BASE_ADDRESS, pages_for_executable);

	//Create main thread
	process->main_thread = CreateThread(process->pdbr, (HANDLE)process, (fThreadFunction) entry_point, 0, true);

	//Add to process list
	LIST_InsertTail(&processes, (LIST_ENTRY*)process);

	//Ok
	return (HANDLE)process;
}

PUBLIC void ExitProcess(HANDLE _process)
{
	//Get process
	PROCESS* p = (PROCESS*) _process;

	//Remove threads from thread list 
	AcquireLock(&threads_lock);
	if((LIST_ENTRY*)p->main_thread == scheduler)
		scheduler = NextThread(scheduler);

	LIST_Remove((LIST_ENTRY*)p->main_thread); //(must synchronize with scheduler)

	ReleaseLock(&threads_lock);
	CloseHandle(p->main_thread);
	
	//Remove address space
	XKY_ADDRESS_SPACE_Free(p->pdbr);

	//Remove form processes list
	LIST_Remove((LIST_ENTRY*)p);

	//Free process
	CloseHandle(_process);
}

PUBLIC bool ProcessInit()
{
	//Initialize process list
	LIST_Init(&processes);

	//Ok
	return true;
}

#endif
