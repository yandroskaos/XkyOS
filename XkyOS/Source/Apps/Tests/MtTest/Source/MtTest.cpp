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
#include "RTLStub.h"
#include "Threads.h"

//==================================DATA======================================//
#pragma data_seg(".data")
//============================================================================//

dword next = 0;

struct PAINT_PARAMETERS
{
	dword	x;
	dword	y;
	dword	width;
	dword	height;
	ARGB	color;
};

WINDOW window;

string DONE = STRING("Done!");

//==================================CODE======================================//
#pragma code_seg(".code")
//============================================================================//

PUBLIC dword Random(IN dword _limit)
{
	next = next * 1103515245 + 12345;
	return((dword)(next/65536) % _limit);
}

PUBLIC void SetSeed(IN dword _seed)
{
	next = _seed;
}

PUBLIC dword PaintThread(IN void* _parameters)
{
	PAINT_PARAMETERS* _paint_data = (PAINT_PARAMETERS*)_parameters;

	for(dword i = 0; i < 1000000; i++)
	{
		dword x = _paint_data->x + Random(_paint_data->width);
		dword y = _paint_data->y + Random(_paint_data->height);

		XKY_WINDOW_SetPixel(window, x, y, _paint_data->color);
	}

	return 0;
}

PRIVATE bool MtTestExceptionHandler(IN INTERRUPT_FRAME* _frame)
{
	return false;
}

PUBLIC void Main()
{
	if(!LoadRTL(0x40000000)) goto _Finish;

	//Alloc window
	window = XKY_WINDOW_Alloc();
	if(!window)	goto _Finish;

	//Set random seed
	SetSeed(XKY_RTC_Seconds() + XKY_TMR_GetTicks());
	
	//Set exception handler for memory failures
	//XKY_EXCEPTION_SetHandler(0x0E, MtTestExceptionHandler);

	//Set-up stacks so thread finds it's parameters on it
	PAINT_PARAMETERS* p1 = (PAINT_PARAMETERS*)HEAP_Alloc(sizeof(PAINT_PARAMETERS));
	p1->x		= 0;
	p1->y		= 0;
	p1->width	= XKY_WINDOW_GetWidth(window)/2;
	p1->height	= XKY_WINDOW_GetHeight(window)/2;
	p1->color	= SRGB(255, 0, 0);

	PAINT_PARAMETERS* p2 = (PAINT_PARAMETERS*)HEAP_Alloc(sizeof(PAINT_PARAMETERS));
	p2->x		= 0;
	p2->y		= XKY_WINDOW_GetHeight(window)/2;
	p2->width	= XKY_WINDOW_GetWidth(window)/2;
	p2->height	= XKY_WINDOW_GetHeight(window)/2;
	p2->color	= SRGB(0, 255, 0);

	PAINT_PARAMETERS* p3 = (PAINT_PARAMETERS*)HEAP_Alloc(sizeof(PAINT_PARAMETERS));
	p3->x		= XKY_WINDOW_GetWidth(window)/2;
	p3->y		= XKY_WINDOW_GetHeight(window)/2;
	p3->width	= XKY_WINDOW_GetWidth(window)/2;
	p3->height	= XKY_WINDOW_GetHeight(window)/2;
	p3->color	= SRGB(0, 0, 255);

	PAINT_PARAMETERS* p4 = (PAINT_PARAMETERS*)HEAP_Alloc(sizeof(PAINT_PARAMETERS));
	p4->x		= XKY_WINDOW_GetWidth(window)/2;
	p4->y		= 0;
	p4->width	= XKY_WINDOW_GetWidth(window)/2;
	p4->height	= XKY_WINDOW_GetHeight(window)/2;
	p4->color	= SRGB(255, 255, 255);


	//Create threads
	ThreadInit();
	HANDLE t1 = CreateThread(PaintThread, p1);
	HANDLE t2 = CreateThread(PaintThread, p2);
	HANDLE t3 = CreateThread(PaintThread, p3);
	HANDLE t4 = CreateThread(PaintThread, p4);

	//Init scheduler iterator
	scheduler = LIST_First(&threads);

	//Start scheduler
	//XKY_CPU_RegisterCallback(XKY_ADDRESS_SPACE_GetCurrent(), MtCpuTimerCallbackFCFS);
	XKY_CPU_RegisterCallback(XKY_ADDRESS_SPACE_GetCurrent(), MtCpuTimerCallbackRoundRobin);

	while(!ThreadHasFinished(t1) || !ThreadHasFinished(t2) || !ThreadHasFinished(t3) || !ThreadHasFinished(t4));

	//All threads finished
	dword width = XKY_WINDOW_GetWidth(window);
	dword height = XKY_WINDOW_GetHeight(window);

	for(dword x = 0; x < width; x++)
	{
		for(dword y = 0; y < height; y++)
		{
			XKY_WINDOW_SetPixel(window, x, y, TRGB(0, 0, 0));
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
