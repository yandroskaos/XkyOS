/******************************************************************************/
/**
* @file		ntoskrnl.cpp
* @brief	Windows Kernel
* This file implements the mini Windows Kernel for XkyOS as guestOS
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

typedef bool (*fXKY_INTERRUPT_SetHandler)	(IN byte _interrupt, IN fInterruptHandler _handler);
typedef void (*fXKY_INTERRUPT_UnsetHandler)	(IN byte _interrupt, IN fInterruptHandler _handler);
IMPORT(XKY_INTERRUPT_SetHandler);
IMPORT(XKY_INTERRUPT_UnsetHandler);

//==================================DATA======================================//
#pragma data_seg(".data")
//============================================================================//
#include "RTL.h"
#include "Lock.h"
#include "Base.h"
#include "Thread.h"
#include "Process.h"

//==================================DATA======================================//
#pragma data_seg(".data")
//============================================================================//
#define WINDOWS_API_SERVICES			0x40
#define WINDOWS_SERVICE_MessageBoxWaiting	1
#define WINDOWS_SERVICE_MessageBox			2
#define WINDOWS_SERVICE_ExitProcess			3

PRIVATE WINDOW window;
PRIVATE ADDRESS_SPACE windows_pdbr;
PRIVATE bool clicked = false;

string message_box_name = STRING("WINDOWS\\MessageBox.exe");
string process_create_error = STRING("Error creating process");

HANDLE the_process = 0;
string message_box_displayed = STRING("MessageBox Displayed");
string message_box_closed = STRING("MessageBox Closed");

#define WINDOW_WIDTH	80
#define WINDOW_HEIGHT	40
ARGB bitmap[WINDOW_WIDTH][WINDOW_HEIGHT];
BOUNDED_STRING(title, 255, 0);
BOUNDED_STRING(message, 255, 0);

//==================================CODE======================================//
#pragma code_seg(".code")
//============================================================================//
PRIVATE void CopyString(string* str, const char* cc)
{
	byte i = 0;
	while(cc[i])
	{
		str->text[i] = cc[i];
		i++;
	}

	str->size = i;
}

PRIVATE void SaveBitmap()
{
	for(dword x = 0; x < 80; x++)
	{
		for(dword y = 0; y < 40; y++)
		{
			bitmap[x][y] = XKY_WINDOW_GetPixel(window, 200 + x, 160 + y);
		}
	}
}

PRIVATE void RedrawBitmap()
{
	for(dword x = 0; x < 80; x++)
	{
		for(dword y = 0; y < 40; y++)
		{
			XKY_WINDOW_SetPixel(window, 200 + x, 160 + y, bitmap[x][y]);
		}
	}
}

PRIVATE void DrawWindow(IN string* _title, IN string* _message)
{
	//Gris debajo
	for(dword x = 0; x < 80; x++)
	{
		for(dword y = 0; y < 30; y++)
		{
			XKY_WINDOW_SetPixel(window, 200 + x, 160 + y, SRGB(128, 128, 128));
		}
	}

	//Azul arriba
	for(dword x = 0; x < 80; x++)
	{
		for(dword y = 30; y < 40; y++)
		{
			XKY_WINDOW_SetPixel(window, 200 + x, 160 + y, SRGB(0, 0, 255));
		}
	}

	//Cuadrado rojo
	for(dword x = 70; x < 80; x++)
	{
		for(dword y = 30; y < 40; y++)
		{
			XKY_WINDOW_SetPixel(window, 200 + x, 160 + y, SRGB(255, 0, 0));
		}
	}

	//Boton
	for(dword x = 30; x < 50; x++)
	{
		for(dword y = 0; y < 10; y++)
		{
			XKY_WINDOW_SetPixel(window, 200 + x, 160 + y, SRGB(200, 200, 200));
		}
	}

	//Print title
	XKY_WINDOW_PrintText(window, 210, 198, SRGB(0, 0, 0), _title);

	//Print text
	XKY_WINDOW_PrintText(window, 210, 184, SRGB(0, 0, 0), _message);

	//Print ok
	string ok = STRING("Ok");

	XKY_WINDOW_PrintText(window, 232, 168, SRGB(0, 0, 0), &ok);

}

PRIVATE void WINDOWS_MouseCallback(IN dword _x, IN dword _y, IN bool _left_clicked, IN bool _right_clicked)
{
	//If right click, execute MessageBox
	if(_right_clicked)
	{
		the_process = CreateProcess(&message_box_name);
		if(!the_process)
		{
			XKY_DEBUG_Message(&process_create_error, SRGB(0, 255, 0));
		}
		return;
	}

	//If left click, Close window or Shutdown
	if(_left_clicked)
	{
		if(the_process)
		{
			XKY_DEBUG_Message(&message_box_closed, SRGB(255, 0, 0));
			RedrawBitmap();
			clicked = true;
		}
		else
		{
			//Dont know why it doesnt work
			//XKY_OS_Finish();
		}
		return;
	}
}

PRIVATE bool WINDOWS_Services(IN INTERRUPT_FRAME* _frame)
{
	//EAX has service index.
	dword service = _frame->eax;
	_frame->eax = 0;

	//Get user mode stack
	dword* stack = (dword*)_frame->esp;

	//Resolve service
	switch(service)
	{
		case WINDOWS_SERVICE_MessageBoxWaiting:
		{
			_frame->eax = clicked;
			break;
		}
		case WINDOWS_SERVICE_MessageBox:
		{
			XKY_DEBUG_Message(&message_box_displayed, SRGB(255, 0, 0));
			CopyString(message, (const char*)stack[1]);
			CopyString(title, (const char*)stack[2]);
			SaveBitmap();
			DrawWindow(title, message);
			clicked = false;
			break;
		}
		case WINDOWS_SERVICE_ExitProcess:
		{
			XKY_ADDRESS_SPACE_SwitchTo(windows_pdbr);

			ExitProcess(the_process);
			the_process = 0;

			clicked = false;
			//Wait for time
			__asm sti
			for(;;);
		}
	}
	return false;
}

PRIVATE bool DrawDesktop(IN WINDOW _window)
{
	#define DESKTOP_IMAGE_ADDRESS 0x10000000
	string desktop_image = STRING("WINDOWS\\Desktop.bmp");

	if(!XKY_LDR_LoadFile(&desktop_image, XKY_ADDRESS_SPACE_GetCurrent(), DESKTOP_IMAGE_ADDRESS))
		return false;

	//Get window metrics
	dword width = XKY_WINDOW_GetWidth(window);
	dword height = XKY_WINDOW_GetHeight(window);

	//Copy BMP File to Background
	//Jump over BMP header
	byte* bmp = ((byte*)DESKTOP_IMAGE_ADDRESS + 0x36);
	for(dword i = 0; i < height; i++)
	{
		for(dword j = 0; j < width; j++)
		{
			byte B = *bmp;
			byte G = *(bmp+1);
			byte R = *(bmp+2);
			XKY_WINDOW_SetPixel(window, j, i, SRGB(R,G,B));
			bmp+=3;
		}
	}

	//Release memory
	dword file_size = XKY_LDR_FileSize(&desktop_image);
	XKY_PAGE_Free(XKY_ADDRESS_SPACE_GetCurrent(), DESKTOP_IMAGE_ADDRESS, RTL_BytesToPages(file_size));

	//Ok
	return true;
}

PUBLIC bool Main()
{
	//Get desktop window
	window = XKY_WINDOW_Alloc();
	if(!window) return false;

	//Get address space
	windows_pdbr = XKY_ADDRESS_SPACE_GetCurrent();

	//Load desktop image
	if(!DrawDesktop(window)) return false;

	//Initialize RTL
	if(!RTL_Init())	return false;

	//Acquire basic resources
	//Get CPU
	//Get Pages
	//Get DISK
	ThreadInit();
	ProcessInit();

	//Register kernel services
	if(!XKY_INTERRUPT_SetHandler(WINDOWS_API_SERVICES, WINDOWS_Services)) return false;

	//Register mouse callback
	XKY_WINDOW_RegisterMouse(window, XKY_ADDRESS_SPACE_GetCurrent(), WINDOWS_MouseCallback);

	//Register CPU callback
	XKY_CPU_RegisterCallback(XKY_ADDRESS_SPACE_GetCurrent(), RoundRobinSchedulingPolicy);

	return true;
}

//=================================EXPORTS====================================//
#pragma data_seg(".exports")
//============================================================================//

//=================================MODULE=====================================//
#pragma data_seg(".module")
//============================================================================//
	MODULE(IMAGE_MODE_SYSTEM, IMAGE_KIND_MODULE, IMAGE_VERSION(1,0,0,0), 0, Main, 0);
