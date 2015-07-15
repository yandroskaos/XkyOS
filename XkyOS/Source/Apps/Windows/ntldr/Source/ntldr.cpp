/******************************************************************************/
/**
* @file		ntldr.cpp
* @brief	Windows Loader and GUI
* This is the starting module of my exokernel based guest OS Windows.
* It will load the windows kernel (ntoskrnl.x).
*
* @date		13/04/2008
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
string ntoskrnl_module	= STRING("WINDOWS\\ntoskrnl.x");

string ntldr_loading = STRING("ntoskrnl loading...");
string ntldr_failure = STRING("ntoskrnl failed loading");
string ntldr_success = STRING("ntoskrnl loaded succesfully");

//==================================CODE======================================//
#pragma code_seg(".code")
//============================================================================//

PUBLIC void Main()
{
	XKY_DEBUG_Message(&ntldr_loading, SRGB(0, 255, 255));

	//Load ntoskrnl
	if(!XKY_LDR_LoadKernelModule(&ntoskrnl_module))
	{
		XKY_DEBUG_Message(&ntldr_failure, SRGB(0, 255, 255));
		XKY_OS_Finish();
	}
	else
	{
		XKY_DEBUG_Message(&ntldr_success, SRGB(0, 255, 255));
		for(;;);
	}
}

//=================================EXPORTS====================================//
#pragma data_seg(".exports")
//============================================================================//

//=================================MODULE=====================================//
#pragma data_seg(".module")
//============================================================================//
	MODULE(IMAGE_MODE_USER, IMAGE_KIND_MODULE, IMAGE_VERSION(1,0,0,0), 0, Main, 0);
