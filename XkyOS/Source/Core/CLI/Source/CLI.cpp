/******************************************************************************/
/**
* @file		CLI.cpp
* @brief	XkyOS User mode Command Line Interpreter
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
#include "RTLStub.h"

//CLI constants
string cli_title = STRING("COMMAND LINE INTERPRETER");
string cli_prompt = STRING("CLI> ");

//KEYBOARD defines
#define VK_SHIFT_LEFT	0x0000002A
#define VK_SHIFT_RIGHT	0x00000036
#define VK_SHIFT_BLOQ	0x0000003A
//#define VK_CTRL_LEFT	0x0000001D
//#define VK_CTRL_RIGHT	0x0000E01D
#define VK_CTRL			0x0000001D
#define VK_ALT			0x00000038
#define VK_SPACE		0x00000039
#define VK_ENTER		0x0000001C
#define VK_BACKSPACE	0x0000000E

#define IS_DEPRESSED(X)		((X)&0x00000080)
#define PURE_SCANCODE(X)	((X)&0xFFFFFF7F)

//WINDOW variables
WINDOW window;

//CLI - CALLBACK variables
bool is_shift_down = false;
bool is_bloqshift_active = false;
bool is_control_down = false;
bool is_alt_down = false;

BOUNDED_STRING(show_key, 1, 1);
BOUNDED_STRING(command_line, 255, 0);
string extension = STRING(".x");

byte n_row[] = {'0', '1', '2', '3', '4', '5', '6', '7', '8', '9'};
byte q_row[] = {'q', 'w', 'e', 'r', 't', 'y', 'u', 'i', 'o', 'p'};
byte Q_row[] = {'Q', 'W', 'E', 'R', 'T', 'Y', 'U', 'I', 'O', 'P'};
byte a_row[] = {'a', 's', 'd', 'f', 'g', 'h', 'j', 'k', 'l'};
byte A_row[] = {'A', 'S', 'D', 'F', 'G', 'H', 'J', 'K', 'L'};
byte z_row[] = {'z', 'x', 'c', 'v', 'b', 'n', 'm'};
byte Z_row[] = {'Z', 'X', 'C', 'V', 'B', 'N', 'M'};

//==================================CODE======================================//
#pragma code_seg(".code")
//============================================================================//
byte KEY_Translate(IN dword _scan_code)
{
	//If extended scan code, ignore
	if(_scan_code == 0x000000E0)
		return 0;

	//Test special key
		//Shift
	if(PURE_SCANCODE(_scan_code) == VK_SHIFT_LEFT || PURE_SCANCODE(_scan_code) == VK_SHIFT_RIGHT)
	{
		if(IS_DEPRESSED(_scan_code))
			is_shift_down = false;
		else
			is_shift_down = true;
		return 0;
	}
		//Bloq-Shift
	if(PURE_SCANCODE(_scan_code) == VK_SHIFT_BLOQ)
	{
		if(IS_DEPRESSED(_scan_code))
			is_bloqshift_active = !is_bloqshift_active;
		return 0;
	}
		//Control
	if(PURE_SCANCODE(_scan_code) == VK_CTRL)
	{
		if(IS_DEPRESSED(_scan_code))
			is_control_down = false;
		else
			is_control_down = true;
		return 0;
	}
		//Alt
	if(PURE_SCANCODE(_scan_code) == VK_ALT)
	{
		if(IS_DEPRESSED(_scan_code))
			is_alt_down = false;
		else
			is_alt_down = true;
		return 0;
	}
	
		
	//Test between 1 and 0
	if(_scan_code >= 0x00000002 && _scan_code <= 0x0000000B)
		return n_row[_scan_code - 0x00000002];

	//Test between Q and P
	if(_scan_code >= 0x00000010 && _scan_code <= 0x00000019)
	{
		if((is_shift_down && !is_bloqshift_active) || (!is_shift_down && is_bloqshift_active))
		{
			return Q_row[_scan_code - 0x00000010];
		}
		else
		{
			return q_row[_scan_code - 0x00000010];
		}
	}

	//Test between A and L
	if(_scan_code >= 0x0000001E && _scan_code <= 0x00000026)
	{
		if((is_shift_down && !is_bloqshift_active) || (!is_shift_down && is_bloqshift_active))
		{
			return A_row[_scan_code - 0x0000001E];
		}
		else
		{
			return a_row[_scan_code - 0x0000001E];
		}

	}

	//Test between Z and M
	if(_scan_code >= 0x0000002C && _scan_code <= 0x00000032)
	{
		if((is_shift_down && !is_bloqshift_active) || (!is_shift_down && is_bloqshift_active))
		{
			return Z_row[_scan_code - 0x0000002C];
		}
		else
		{
			return z_row[_scan_code - 0x0000002C];
		}
	}

	return 0;
}

void ConsoleKeyboardCallback(IN dword _scan_code)
{
	//Check for ENTER
	if(_scan_code == PURE_SCANCODE(VK_ENTER))
	{
		if(!IS_DEPRESSED(_scan_code))
		{
			CONSOLE_NewLine();
			if(command_line->size)
			{
				//Try to execute
				STRING_Append(command_line, &extension);
				if(XKY_OS_Start(command_line))
				{
					string success = STRING("Success launching ");
					CONSOLE_Write(&success, SRGB(0, 0, 255));
					CONSOLE_WriteLn(command_line, SRGB(0, 0, 255));
				}
				else
				{
					string failure = STRING("Failure launching ");
					CONSOLE_Write(&failure, SRGB(0, 0, 255));
					CONSOLE_WriteLn(command_line, SRGB(0, 0, 255));
				}
			}
			CONSOLE_Write(&cli_prompt, SRGB(0, 255, 0));
			command_line->size = 0;
		}
		return;
	}

	//Check for BACKSPACE
	if(_scan_code == PURE_SCANCODE(VK_BACKSPACE))
	{
		if(!IS_DEPRESSED(_scan_code))
		{
			if(command_line->size)
			{
				CONSOLE_SetCursor(CONSOLE_GetX() - 8, CONSOLE_GetY());
				show_key->text[0] = 0;
				CONSOLE_Write(show_key, TRGB(200, 0, 0));
				CONSOLE_SetCursor(CONSOLE_GetX() - 8, CONSOLE_GetY());
				command_line->size--;
			}
		}
		return;
	}

	//Check for SPACE
	if(_scan_code == PURE_SCANCODE(VK_SPACE))
	{
		if(!IS_DEPRESSED(_scan_code))
		{
			command_line->text[command_line->size++] = '\\';
			show_key->text[0] = 0x5C;
			CONSOLE_Write(show_key, SRGB(0, 255, 0));
		}
		return;
	}

	//Rest of scan codes
	byte key = KEY_Translate(_scan_code);
	if(key)
	{
		command_line->text[command_line->size++] = key;
		show_key->text[0] = key;
		CONSOLE_Write(show_key, SRGB(0, 255, 0));
	}
}

PUBLIC void Main()
{
	//Ubicamos una ventana
	window = XKY_WINDOW_Alloc();
	if(!window) goto _Error;

    //Cargamos la RTL
	if(!LoadRTL(0x40000000)) goto _Error;
	
	//Iniciamos la consola
	CONSOLE_Init(window, TRGB(200, 0, 0), &cli_title, &cli_prompt);

	//Registramos el keyboard
	XKY_WINDOW_RegisterKeyboard(window, XKY_ADDRESS_SPACE_GetCurrent(), ConsoleKeyboardCallback);

	for(;;);

	//Done
_Error:
	XKY_OS_Finish();
}

//=================================EXPORTS====================================//
#pragma data_seg(".exports")
//============================================================================//

//=================================MODULE=====================================//
#pragma data_seg(".module")
//============================================================================//
	MODULE(IMAGE_MODE_USER, IMAGE_KIND_MODULE, IMAGE_VERSION(1,0,0,0), 0, Main, 0);
