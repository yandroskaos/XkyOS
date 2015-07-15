/******************************************************************************/
/**
* @file	Hello.cpp
* @brief	PUT HERE YOUR SHORT DESCRIPTION
* PUT HERE YOUR LONG DESCRIPTION
*
* @date	PUT HERE THE DATE
* @author	PUT HERE YOUR NAME
*/
/******************************************************************************/
#include "Types.h"
#include "Executable.h"

//PUT HERE YOUR ADDITONAL INCLUDES

//=================================IMPORTS====================================//
#pragma data_seg(".imports")
//============================================================================//
#include "API.h"

//==================================DATA======================================//
#pragma data_seg(".data")
//============================================================================//
//PUT HERE YOUR GLOBAL VARIABLES

//==================================CODE======================================//
#pragma code_seg(".code")
//============================================================================//
//PUT HERE YOUR CODE

PUBLIC void Main()
{
	//PUT HERE YOUR CODE
	string hello_world = STRING("Hello world!");
	XKY_DEBUG_Message(&hello_world, SRGB(255, 0, 0));
	XKY_OS_Finish();
}

//=================================EXPORTS====================================//
#pragma data_seg(".exports")
//============================================================================//

//=================================MODULE=====================================//
#pragma data_seg(".module")
//============================================================================//
	MODULE(IMAGE_MODE_USER, IMAGE_KIND_MODULE, IMAGE_VERSION(1,0,0,0), 0, Main, 0);
