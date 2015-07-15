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

PRIVATE bool thread1_running = true;
PRIVATE bool thread2_running = true;
PRIVATE bool thread3_running = true;
PRIVATE bool thread4_running = true;

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
	for(dword i = 0; i < 100; i++)
	{
		dword x = _paint_data.x + Random(_paint_data.width);
		dword y = _paint_data.y + Random(_paint_data.height);

		XKY_WINDOW_SetPixel(window, x, y, _paint_data.color);
	}
	
	*_paint_data.running = false;
	XKY_CPU_Free(_paint_data.xid);
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
	XKY_EXCEPTION_SetHandler(0x0E, MtTestExceptionHandler);

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
	PAINT_PARAMETERS* p1 = (PAINT_PARAMETERS*) (stack1 + PAGE_SIZE - sizeof(PAINT_PARAMETERS) - 4);
	p1->running	= &thread1_running;
	p1->x		= 0;
	p1->y		= 0;
	p1->width	= XKY_WINDOW_GetWidth(window)/2;
	p1->height	= XKY_WINDOW_GetHeight(window)/2;
	p1->color	= SRGB(255, 0, 0);

	PAINT_PARAMETERS* p2 = (PAINT_PARAMETERS*) (stack2 + PAGE_SIZE - sizeof(PAINT_PARAMETERS) - 4);
	p2->running	= &thread2_running;
	p2->x		= 0;
	p2->y		= XKY_WINDOW_GetHeight(window)/2;
	p2->width	= XKY_WINDOW_GetWidth(window)/2;
	p2->height	= XKY_WINDOW_GetHeight(window)/2;
	p2->color	= SRGB(0, 255, 0);

	PAINT_PARAMETERS* p3 = (PAINT_PARAMETERS*) (stack3 + PAGE_SIZE - sizeof(PAINT_PARAMETERS) - 4);
	p3->running	= &thread3_running;
	p3->x		= XKY_WINDOW_GetWidth(window)/2;
	p3->y		= XKY_WINDOW_GetHeight(window)/2;
	p3->width	= XKY_WINDOW_GetWidth(window)/2;
	p3->height	= XKY_WINDOW_GetHeight(window)/2;
	p3->color	= SRGB(0, 0, 255);

	PAINT_PARAMETERS* p4 = (PAINT_PARAMETERS*) (stack4 + PAGE_SIZE - sizeof(PAINT_PARAMETERS) - 4);
	p4->running	= &thread4_running;
	p4->x		= XKY_WINDOW_GetWidth(window)/2;
	p4->y		= 0;
	p4->width	= XKY_WINDOW_GetWidth(window)/2;
	p4->height	= XKY_WINDOW_GetHeight(window)/2;
	p4->color	= SRGB(255, 255, 255);

	//Create threads
	if(!(p1->xid = XKY_CPU_AllocCode((byte*)PaintThread, (VIRTUAL)((byte*)p1 - 4)))) goto _Finish;
	if(!(p2->xid = XKY_CPU_AllocCode((byte*)PaintThread, (VIRTUAL)((byte*)p2 - 4)))) goto _Finish;
	if(!(p3->xid = XKY_CPU_AllocCode((byte*)PaintThread, (VIRTUAL)((byte*)p3 - 4)))) goto _Finish;
	if(!(p4->xid = XKY_CPU_AllocCode((byte*)PaintThread, (VIRTUAL)((byte*)p4 - 4)))) goto _Finish;

    //Done, stop this thread
	XKY_WINDOW_PrintText(window, 100, 100, SRGB(255, 0, 255), &OK);
	for(;thread1_running || thread2_running || thread3_running || thread4_running;);

	//All threads finished
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
