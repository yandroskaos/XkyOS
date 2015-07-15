/******************************************************************************/
/**
* @file		MtTest.cpp
* @brief	Multi-Threaded test application
* Test application with multiple paths of execution.
*
* @date		10/04/2008
* @author	Pablo Bravo García
*/
/******************************************************************************/
#include "Types.h"
#include "Executable.h"

//=================================IMPORTS====================================//
#pragma data_seg(".imports")
//============================================================================//
#include "API.h"

//==================================DATA======================================//
#pragma data_seg(".data")
//============================================================================//

struct PAINT_PARAMETERS
{
	bool*	running;
	CPU		xid;
	dword	x;
	dword	y;
	dword	width;
	dword	height;
	ARGB	color;
};

PRIVATE WINDOW window;
PRIVATE dword next = 1;
PRIVATE string OK = STRING("Threads Launched!");
PRIVATE string DONE = STRING("Threads Finished!");

PRIVATE bool threads_done = false;
PRIVATE bool thread1_running = true;
PRIVATE bool thread2_running = true;
PRIVATE bool thread3_running = true;
PRIVATE bool thread4_running = true;

PRIVATE PAINT_PARAMETERS* p1;
PRIVATE PAINT_PARAMETERS* p2;
PRIVATE PAINT_PARAMETERS* p3;
PRIVATE PAINT_PARAMETERS* p4;

PRIVATE bool first_time = true;
PRIVATE CPU_EXECUTION original;
PRIVATE CPU_EXECUTION c1;
PRIVATE CPU_EXECUTION c2;
PRIVATE CPU_EXECUTION c3;
PRIVATE CPU_EXECUTION c4;
PRIVATE CPU_EXECUTION* current_execution;

PRIVATE string ThreadDone = STRING("Thread Finished = ");
PRIVATE string Rescheduling1 = STRING("Original Thread Rescheduled 1");
PRIVATE string Rescheduling2 = STRING("Original Thread Rescheduled 2");
PRIVATE string Rescheduling3 = STRING("Original Thread Rescheduled 3");

//==================================CODE======================================//
#pragma code_seg(".code")
//============================================================================//
PRIVATE dword Random(IN dword _limit)
{
	next = next * 1103515245 + 12345;
	return((dword)(next/65536) % _limit);
}

PRIVATE void SetSeed(IN dword _seed)
{
	next = _seed;
}

PUBLIC void PaintThread(IN PAINT_PARAMETERS _paint_data)
{
	for(dword i = 0; i < 10000; i++)
	//for(dword i = 0; i < 1; i++)
	{
		dword x = _paint_data.x + Random(_paint_data.width);
		dword y = _paint_data.y + Random(_paint_data.height);

		XKY_WINDOW_SetPixel(window, x, y, _paint_data.color);
	}
	

	XKY_DEBUG_Data(&ThreadDone, _paint_data.xid, _paint_data.color);
	*_paint_data.running = false;

	for(;;);
}

//FCFS
PRIVATE bool MtCpuTimerCallbackFCFS(IN CPU_EXECUTION* _execution, IN dword _interrupt_sequence, IN dword _max_interrupt_per_sequence)
{
	//Ok, start
	if(first_time)
	{
		first_time = false;
		//Save
		original = *_execution;

		//Initialize
		c1 = *_execution;
		c2 = *_execution;
		c3 = *_execution;
		c4 = *_execution;

		c1.eax = c2.eax = c3.eax = c4.eax = 0;
		c1.ebx = c2.ebx = c3.ebx = c4.ebx = 0;
		c1.ecx = c2.ecx = c3.ecx = c4.ecx = 0;
		c1.edx = c2.edx = c3.edx = c4.edx = 0;
		c1.esi = c2.esi = c3.esi = c4.esi = 0;
		c1.edi = c2.edi = c3.edi = c4.edi = 0;
		c1.ebp = c2.ebp = c3.ebp = c4.ebp = 0;

		c1.esp = (VIRTUAL)((byte*)p1 - 4);
		c2.esp = (VIRTUAL)((byte*)p2 - 4);
		c3.esp = (VIRTUAL)((byte*)p3 - 4);
		c4.esp = (VIRTUAL)((byte*)p4 - 4);

		c1.eip = c2.eip = c3.eip = c4.eip = (dword)PaintThread;
		c1.eflags = c2.eflags = c3.eflags = c4.eflags = 0x00000202;

		XKY_DEBUG_Data(&Rescheduling1, original.eip, SRGB(255, 0, 255));
		return false;
	}

	if(!thread1_running && !thread2_running && !thread3_running && !thread4_running)
	{
		if(!threads_done)
		{
			threads_done = true;
			*_execution = original;
			XKY_DEBUG_Data(&Rescheduling2, original.eip, SRGB(255, 0, 255));
			return true;
		}
		XKY_DEBUG_Data(&Rescheduling3, _execution->eip, SRGB(255, 0, 255));
		return false;
	}

	*current_execution = *_execution;

	if(current_execution == &c1)
	{
		if(thread1_running)
			return false;
		else
			current_execution = &c2;
	}
	else if(current_execution == &c2)
	{
		if(thread2_running)
			return false;
		else
			current_execution = &c3;
	}
	else if(current_execution == &c3)
	{
		if(thread3_running)
			return false;
		else
			current_execution = &c4;
	}
	else if(current_execution == &c4)
	{	
		if(thread4_running)
			return false;
		else
			current_execution = &original;
	}
	else
	{
		if(thread1_running)
		{
			current_execution = &c1;
		}
	}

	*_execution = *current_execution;
	return true; //Means another thread scheduled
}


//ROUND-ROBIN
PRIVATE bool MtCpuTimerCallbackRoundRobin(IN CPU_EXECUTION* _execution, IN dword _interrupt_sequence, IN dword _max_interrupt_per_sequence)
{
	//Ok, start
	if(first_time)
	{
		first_time = false;
		//Save
		original = *_execution;

		//Initialize
		c1 = *_execution;
		c2 = *_execution;
		c3 = *_execution;
		c4 = *_execution;

		c1.eax = c2.eax = c3.eax = c4.eax = 0;
		c1.ebx = c2.ebx = c3.ebx = c4.ebx = 0;
		c1.ecx = c2.ecx = c3.ecx = c4.ecx = 0;
		c1.edx = c2.edx = c3.edx = c4.edx = 0;
		c1.esi = c2.esi = c3.esi = c4.esi = 0;
		c1.edi = c2.edi = c3.edi = c4.edi = 0;
		c1.ebp = c2.ebp = c3.ebp = c4.ebp = 0;

		c1.esp = (VIRTUAL)((byte*)p1 - 4);
		c2.esp = (VIRTUAL)((byte*)p2 - 4);
		c3.esp = (VIRTUAL)((byte*)p3 - 4);
		c4.esp = (VIRTUAL)((byte*)p4 - 4);

		c1.eip = c2.eip = c3.eip = c4.eip = (dword)PaintThread;
		c1.eflags = c2.eflags = c3.eflags = c4.eflags = 0x00000202;

		XKY_DEBUG_Data(&Rescheduling1, original.eip, SRGB(255, 0, 255));
		return false;
	}

	if(!thread1_running && !thread2_running && !thread3_running && !thread4_running)
	{
		if(!threads_done)
		{
			threads_done = true;
			*_execution = original;
			XKY_DEBUG_Data(&Rescheduling2, original.eip, SRGB(255, 0, 255));
			return true;
		}
		XKY_DEBUG_Data(&Rescheduling3, _execution->eip, SRGB(255, 0, 255));
		return false;
	}

	*current_execution = *_execution;

	if(current_execution == &c1)
	{
		current_execution = &c2;
	}
	else if(current_execution == &c2)
	{
		current_execution = &c3;
	}
	else if(current_execution == &c3)
	{
		current_execution = &c4;
	}
	else
	{	
		current_execution = &c1;
	}

	*_execution = *current_execution;
	return true; //Means another thread scheduled
}

PRIVATE bool MtTestExceptionHandler(IN INTERRUPT_FRAME* _frame)
{
	return false;
}

PUBLIC void Main()
{
	//Alloc window
	window = XKY_WINDOW_Alloc();
	if(!window)	goto _Finish;

	//Set random seed
	SetSeed(XKY_RTC_Seconds());
	
	//Set exception handler for memory failures
	//XKY_EXCEPTION_SetHandler(0x0E, MtTestExceptionHandler);

	//Alloc stacks
	byte* stack1 = XKY_PAGE_Alloc(0x10000000, 1);
	if(!stack1) goto _Finish;

	byte* stack2 = XKY_PAGE_Alloc(0x20000000, 1);
	if(!stack2) goto _Finish;

	byte* stack3 = XKY_PAGE_Alloc(0x30000000, 1);
	if(!stack3) goto _Finish;

	byte* stack4 = XKY_PAGE_Alloc(0x40000000, 1);
	if(!stack4) goto _Finish;

	//Set-up stacks so thread finds it's parameters on it
	p1 = (PAINT_PARAMETERS*) (stack1 + PAGE_SIZE - sizeof(PAINT_PARAMETERS) - 4);
	p1->xid		= 1;
	p1->running	= &thread1_running;
	p1->x		= 0;
	p1->y		= 0;
	p1->width	= XKY_WINDOW_GetWidth(window)/2;
	p1->height	= XKY_WINDOW_GetHeight(window)/2;
	p1->color	= SRGB(255, 0, 0);

	p2 = (PAINT_PARAMETERS*) (stack2 + PAGE_SIZE - sizeof(PAINT_PARAMETERS) - 4);
	p2->xid		= 2;
	p2->running	= &thread2_running;
	p2->x		= 0;
	p2->y		= XKY_WINDOW_GetHeight(window)/2;
	p2->width	= XKY_WINDOW_GetWidth(window)/2;
	p2->height	= XKY_WINDOW_GetHeight(window)/2;
	p2->color	= SRGB(0, 255, 0);

	p3 = (PAINT_PARAMETERS*) (stack3 + PAGE_SIZE - sizeof(PAINT_PARAMETERS) - 4);
	p3->xid		= 3;
	p3->running	= &thread3_running;
	p3->x		= XKY_WINDOW_GetWidth(window)/2;
	p3->y		= XKY_WINDOW_GetHeight(window)/2;
	p3->width	= XKY_WINDOW_GetWidth(window)/2;
	p3->height	= XKY_WINDOW_GetHeight(window)/2;
	p3->color	= SRGB(0, 0, 255);

	p4 = (PAINT_PARAMETERS*) (stack4 + PAGE_SIZE - sizeof(PAINT_PARAMETERS) - 4);
	p4->xid		= 4;
	p4->running	= &thread4_running;
	p4->x		= XKY_WINDOW_GetWidth(window)/2;
	p4->y		= 0;
	p4->width	= XKY_WINDOW_GetWidth(window)/2;
	p4->height	= XKY_WINDOW_GetHeight(window)/2;
	p4->color	= SRGB(255, 255, 255);

	
	//Create threads
	XKY_CPU_RegisterCallback(XKY_CPU_GetCurrent(), MtCpuTimerCallbackFCFS);

    //Done, stop this thread
	XKY_WINDOW_PrintText(window, 100, 100, SRGB(255, 0, 255), &OK);
	for(;thread1_running || thread2_running || thread3_running || thread4_running;);

	//All threads finished
	dword width = XKY_WINDOW_GetWidth(window);
	dword height = XKY_WINDOW_GetHeight(window);
	for(dword x = 0; x < width; x++)
	{
		for(dword y = 0; y < height; y++)
		{
			XKY_WINDOW_SetPixel(window, x, y, SRGB(0, 0, 0));
		}
	}

	XKY_WINDOW_PrintText(window, 200, 200, SRGB(255, 0, 255), &DONE);

_Finish:
	XKY_OS_Finish();
}

//=================================EXPORTS====================================//
#pragma data_seg(".exports")
//============================================================================//

//=================================MODULE=====================================//
#pragma data_seg(".module")
//============================================================================//
	MODULE(IMAGE_MODE_USER, IMAGE_KIND_MODULE, IMAGE_VERSION(1,0,0,0), 0, Main, 0);
