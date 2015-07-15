/******************************************************************************/
/**
* @file		Test.cpp
* @brief	XkyOS User mode test application
* Example of a working XkyOS application
* 
* @date		20/03/2008
* @author	Pablo Bravo
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
PRIVATE WINDOW window;

//==================================CODE======================================//
#pragma code_seg(".code")
//============================================================================//

PRIVATE bool TestExceptionHandler(IN INTERRUPT_FRAME* _frame)
{
	string unhandled = STRING("User Unhandled exception!");

	XKY_WINDOW_PrintText(window, 100, 100, SRGB(0,255,255), &unhandled);

	return false;
}

PUBLIC void Main()
{
	window = XKY_WINDOW_Alloc();
	if(window)
	{
		SetSeed(XKY_RTC_Seconds());

		XKY_EXCEPTION_SetHandler(0x0E, TestExceptionHandler);

		//This will provoke a page fault
		*(dword*)0 = 0;
	}
	XKY_OS_Finish();
}

//=================================EXPORTS====================================//
#pragma data_seg(".exports")
//============================================================================//

//=================================MODULE=====================================//
#pragma data_seg(".module")
//============================================================================//
	MODULE(IMAGE_MODE_USER, IMAGE_KIND_MODULE, IMAGE_VERSION(1,0,0,0), 0, Main, 0);
