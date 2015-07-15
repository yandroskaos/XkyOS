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
PRIVATE string console_name = STRING("COMMONS\\RTL.x");
PRIVATE string HELLO = STRING("Hello Console");

PRIVATE string OK1 = STRING("1");
PRIVATE string OK2 = STRING("2");
PRIVATE string OK3 = STRING("3");

//
typedef void	(*fCONSOLE_Init)		(IN WINDOW _window, IN ARGB _color);
typedef void	(*fCONSOLE_SetCursor)	(IN dword _new_x, IN dword _new_y);
typedef dword	(*fCONSOLE_GetX)		();
typedef dword	(*fCONSOLE_GetY)		();
typedef void	(*fCONSOLE_Clear)		(IN ARGB _color);
typedef void	(*fCONSOLE_Write)		(IN string* _text, IN ARGB _color);
typedef void	(*fCONSOLE_NewLine)		();
typedef void	(*fCONSOLE_WriteLn)		(IN string* _text, IN ARGB _color);
typedef void	(*fCONSOLE_WriteNumber)	(IN dword _data, IN byte _size, IN ARGB _color);
//
fCONSOLE_Init			CONSOLE_Init		= 0;
fCONSOLE_SetCursor		CONSOLE_SetCursor	= 0;
fCONSOLE_GetX			CONSOLE_GetX		= 0;
fCONSOLE_GetY			CONSOLE_GetY		= 0;
fCONSOLE_Clear			CONSOLE_Clear		= 0;
fCONSOLE_Write			CONSOLE_Write		= 0;
fCONSOLE_NewLine		CONSOLE_NewLine		= 0;
fCONSOLE_WriteLn		CONSOLE_WriteLn		= 0;
fCONSOLE_WriteNumber	CONSOLE_WriteNumber	= 0;
//
string CONSOLE_Init_name		= STRING("CONSOLE_Init");
string CONSOLE_SetCursor_name	= STRING("CONSOLE_SetCursor");
string CONSOLE_GetX_name		= STRING("CONSOLE_GetX");
string CONSOLE_GetY_name		= STRING("CONSOLE_GetY");
string CONSOLE_Clear_name		= STRING("CONSOLE_Clear");
string CONSOLE_Write_name		= STRING("CONSOLE_Write");
string CONSOLE_NewLine_name		= STRING("CONSOLE_NewLine");
string CONSOLE_WriteLn_name		= STRING("CONSOLE_WriteLn");
string CONSOLE_WriteNumber_name	= STRING("CONSOLE_WriteNumber");

//==================================CODE======================================//
#pragma code_seg(".code")
//============================================================================//

PUBLIC void Main()
{
	window = XKY_WINDOW_Alloc();
	if(window)
	{
		if(XKY_LDR_LoadUserModule(&console_name, 0x10000000))
		{
			CONSOLE_Init		= (fCONSOLE_Init)		XKY_LDR_GetProcedureAddress(0x10000000, &CONSOLE_Init_name);
			CONSOLE_SetCursor	= (fCONSOLE_SetCursor)	XKY_LDR_GetProcedureAddress(0x10000000, &CONSOLE_SetCursor_name);
			CONSOLE_GetX		= (fCONSOLE_GetX)		XKY_LDR_GetProcedureAddress(0x10000000, &CONSOLE_GetX_name);
			CONSOLE_GetY		= (fCONSOLE_GetY)		XKY_LDR_GetProcedureAddress(0x10000000, &CONSOLE_GetY_name);
			CONSOLE_Clear		= (fCONSOLE_Clear)		XKY_LDR_GetProcedureAddress(0x10000000, &CONSOLE_Clear_name);
			CONSOLE_Write		= (fCONSOLE_Write)		XKY_LDR_GetProcedureAddress(0x10000000, &CONSOLE_Write_name);
			CONSOLE_NewLine		= (fCONSOLE_NewLine)	XKY_LDR_GetProcedureAddress(0x10000000, &CONSOLE_NewLine_name);
			CONSOLE_WriteLn		= (fCONSOLE_WriteLn)	XKY_LDR_GetProcedureAddress(0x10000000, &CONSOLE_WriteLn_name);
			CONSOLE_WriteNumber	= (fCONSOLE_WriteNumber)XKY_LDR_GetProcedureAddress(0x10000000, &CONSOLE_WriteNumber_name);


			if(CONSOLE_Init)
			{
				CONSOLE_Init(window, TRGB(0, 255, 0));
				CONSOLE_SetCursor(5, 5);
				CONSOLE_WriteLn(&HELLO, SRGB(255, 0, 255));
			}
		}
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
