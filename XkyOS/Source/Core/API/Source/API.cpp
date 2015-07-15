/******************************************************************************/
/**
* @file		API.cpp
* @brief	XkyOS Services user-mode stub module
* Module loaded in user-mode environment addresses to allow to communicate with the kernel.
* 
* @date		20/03/2008
* @author	Pablo Bravo
*/
/******************************************************************************/
#include "Types.h"
#include "Executable.h"
#include "Image.h"

#include "Functions.h"

//=================================IMPORTS====================================//
#pragma data_seg(".imports")
//============================================================================//

//==================================DATA======================================//
#pragma data_seg(".data")
//============================================================================//
typedef dword ADDRESS_SPACE;
typedef dword VIRTUAL;
typedef dword LBA;
typedef dword WINDOW;
typedef dword ARGB;
typedef dword POINTER;
typedef dword PCI;
typedef dword XID;
struct EXECUTION
{
	//Selectors
	dword gs;
	dword fs;
	dword es;
	dword ds;
	dword cs;
	dword ss;
	//General purpose registers
	dword edi;
	dword esi;
	dword ebp;
	dword esp;
	dword ebx;
	dword edx;
	dword ecx;
	dword eax;
	//Eip
	dword eip;
	//Flags
	dword eflags;
	//PDBR
	dword pdbr;
};
typedef bool (*fCpuTimerCallback)(IN XID _xid, IN EXECUTION* _execution);
typedef void (*fWindowMouseCallback)(IN dword _x, IN dword _y, IN bool _left_clicked, IN bool _right_clicked);
typedef void (*fWindowKeyboardCallback)(IN dword _scan_code);
enum DaysOfWeek
{
	Sunday = 1,
	Monday,
	Tuesday,
	Wednesday,
	Thursday,
	Friday,
	Saturday
};
struct INTERRUPT_FRAME
{
	//Selectors
	dword gs;
	dword fs;
	dword es;
	dword ds;
	//General purpose registers
	dword edi;
	dword esi;
	dword ebp;
	dword kernel_esp;
	dword ebx;
	dword edx;
	dword ecx;
	dword eax;
	//Interrupt vector
	dword vector;
	//Error if exception with error
	dword error;
	//Code cs:eip to return when interrupt finishes servicing
	dword eip;
	dword cs;
	//Flags
	dword eflags;
	//Stack ss:esp
	dword esp;
	dword ss;
};
typedef bool (*fInterruptHandler)(IN INTERRUPT_FRAME* _frame);

//==================================CODE======================================//
#pragma code_seg(".code")
//============================================================================//
PUBLIC bool API_EntryPoint()
{
	return true;
}

#define CALL0(X)	__asm mov eax, X \
					__asm int OS_API_SERVICES \
					__asm ret 

#define CALL1(X)	__asm push ebp \
					__asm mov ebp, esp \
					__asm push dword ptr [ebp + 8] \
					__asm mov eax, X \
					__asm int OS_API_SERVICES \
					__asm add esp, 4 \
					__asm pop ebp \
					__asm ret 4

#define CALL2(X)	__asm push ebp \
					__asm mov ebp, esp \
					__asm push dword ptr [ebp + 12] \
					__asm push dword ptr [ebp + 8] \
					__asm mov eax, X \
					__asm int OS_API_SERVICES \
					__asm add esp, 8 \
					__asm pop ebp \
					__asm ret 8

#define CALL3(X)	__asm push ebp \
					__asm mov ebp, esp \
					__asm push dword ptr [ebp + 16] \
					__asm push dword ptr [ebp + 12] \
					__asm push dword ptr [ebp + 8] \
					__asm mov eax, X \
					__asm int OS_API_SERVICES \
					__asm add esp, 12 \
					__asm pop ebp \
					__asm ret 12

#define CALL4(X)	__asm push ebp \
					__asm mov ebp, esp \
					__asm push dword ptr [ebp + 20] \
					__asm push dword ptr [ebp + 16] \
					__asm push dword ptr [ebp + 12] \
					__asm push dword ptr [ebp + 8] \
					__asm mov eax, X \
					__asm int OS_API_SERVICES \
					__asm add esp, 16 \
					__asm pop ebp \
					__asm ret 16

#define CALL5(X)	__asm push ebp \
					__asm mov ebp, esp \
					__asm push dword ptr [ebp + 24] \
					__asm push dword ptr [ebp + 20] \
					__asm push dword ptr [ebp + 16] \
					__asm push dword ptr [ebp + 12] \
					__asm push dword ptr [ebp + 8] \
					__asm mov eax, X \
					__asm int OS_API_SERVICES \
					__asm add esp, 20 \
					__asm pop ebp \
					__asm ret 20

//Memory
PUBLIC NAKED ADDRESS_SPACE XKY_ADDRESS_SPACE_Alloc()
{
	CALL0(IDX_XKY_ADDRESS_SPACE_Alloc);
}

PUBLIC NAKED bool XKY_ADDRESS_SPACE_Free(IN ADDRESS_SPACE _address_space)
{
	CALL1(IDX_XKY_ADDRESS_SPACE_Free);
}

PUBLIC NAKED ADDRESS_SPACE XKY_ADDRESS_SPACE_GetCurrent()
{
	CALL0(IDX_XKY_ADDRESS_SPACE_GetCurrent)
}

PUBLIC NAKED bool XKY_ADDRESS_SPACE_SwitchTo(IN ADDRESS_SPACE _address_space)
{
	CALL1(IDX_XKY_ADDRESS_SPACE_SwitchTo)
}

PUBLIC NAKED VIRTUAL XKY_PAGE_Alloc(IN ADDRESS_SPACE _address_space, IN VIRTUAL _address, IN dword _number_of_pages)
{
	CALL3(IDX_XKY_PAGE_Alloc)
}

PUBLIC NAKED void XKY_PAGE_Free(IN ADDRESS_SPACE _address_space, IN VIRTUAL _address, IN dword number_of_pages)
{
	CALL2(IDX_XKY_PAGE_Free)
}

PUBLIC NAKED bool XKY_PAGE_Share(IN ADDRESS_SPACE _pdbr_origin, IN VIRTUAL _origin, IN ADDRESS_SPACE _pdbr_destiny, IN VIRTUAL _destiny, IN dword number_of_pages)
{
	CALL5(IDX_XKY_PAGE_Share)
}


//Disk
PUBLIC NAKED LBA XKY_DISK_Alloc(IN LBA _sector, IN dword _number_of_sectors)
{
	CALL2(IDX_XKY_DISK_Alloc)
}

PUBLIC NAKED void XKY_DISK_Free(IN LBA _sector, IN dword _number_of_sectors)
{
	CALL2(IDX_XKY_DISK_Free)
}

PUBLIC NAKED bool XKY_DISK_Read(IN LBA _sector, IN VIRTUAL _memory, IN dword _number_of_sectors)
{
	CALL3(IDX_XKY_DISK_Read)
}

PUBLIC NAKED bool XKY_DISK_Write(IN LBA _sector, IN VIRTUAL _memory, IN dword _number_of_sectors)
{
	CALL3(IDX_XKY_DISK_Write)
}

//Windows
PUBLIC NAKED WINDOW XKY_WINDOW_Alloc()
{
	CALL0(IDX_XKY_WINDOW_Alloc)
}

PUBLIC NAKED void XKY_WINDOW_Free(IN WINDOW _window)
{
	CALL1(IDX_XKY_WINDOW_Free)
}

PUBLIC NAKED dword XKY_WINDOW_GetHeight(IN WINDOW _window)
{
	CALL1(IDX_XKY_WINDOW_GetHeight)
}

PUBLIC NAKED dword XKY_WINDOW_GetWidth(IN WINDOW _window)
{
	CALL1(IDX_XKY_WINDOW_GetWidth)
}

PUBLIC NAKED ARGB XKY_WINDOW_GetPixel(IN WINDOW _window, IN dword _x, IN dword _y)
{
	CALL3(IDX_XKY_WINDOW_GetPixel)
}

PUBLIC NAKED void XKY_WINDOW_SetPixel(IN WINDOW _window, IN dword _x, IN dword _y, IN ARGB _color)
{
	CALL4(IDX_XKY_WINDOW_SetPixel)
}

PUBLIC NAKED void XKY_WINDOW_PrintText(IN WINDOW _window, IN dword _x, IN dword _y, IN ARGB _color, IN string* _text)
{
	CALL5(IDX_XKY_WINDOW_PrintText)
}

PUBLIC NAKED void XKY_WINDOW_RegisterKeyboard(IN WINDOW _window, IN ADDRESS_SPACE _pdbr, IN fWindowKeyboardCallback _callback)
{
	CALL3(IDX_XKY_WINDOW_RegisterKeyboard)
}

PUBLIC NAKED void XKY_WINDOW_RegisterMouse(IN WINDOW _window, IN ADDRESS_SPACE _pdbr, IN fWindowMouseCallback _callback)
{
	CALL3(IDX_XKY_WINDOW_RegisterMouse)
}

PUBLIC NAKED POINTER XKY_POINTER_GetArrow()
{
	CALL0(IDX_XKY_POINTER_GetArrow)
}

PUBLIC NAKED POINTER XKY_POINTER_GetClock()
{
	CALL0(IDX_XKY_POINTER_GetClock)	
}

PUBLIC NAKED void XKY_WINDOW_SetPointer(IN WINDOW _window, IN POINTER _pointer)
{
	CALL2(IDX_XKY_WINDOW_SetPointer)
}

PUBLIC NAKED void XKY_WINDOW_SetPointerColor(IN WINDOW _window, IN ARGB _color)
{
	CALL2(IDX_XKY_WINDOW_SetPointerColor)
}

//PCI
PUBLIC NAKED PCI XKY_PCI_Alloc(IN PCI _device)
{
	CALL1(IDX_XKY_PCI_Alloc)
}

PUBLIC NAKED void XKY_PCI_Free(IN PCI _device)
{
	CALL1(IDX_XKY_PCI_Free)
}

PUBLIC NAKED dword XKY_PCI_GetNumberOfDevices()
{
	CALL0(IDX_XKY_PCI_GetNumberOfDevices)
}

PUBLIC NAKED PCI XKY_PCI_GetDevice(IN dword _index)
{
	CALL1(IDX_XKY_PCI_GetDevice)
}

PUBLIC NAKED byte XKY_PCI_GetBusFromDirection(IN PCI _device)
{
	CALL1(IDX_XKY_PCI_GetBusFromDirection)
}

PUBLIC NAKED byte XKY_PCI_GetDeviceFromDirection(IN PCI _device)
{
	CALL1(IDX_XKY_PCI_GetDeviceFromDirection)
}

PUBLIC NAKED byte XKY_PCI_GetFunctionFromDirection(IN PCI _device)
{
	CALL1(IDX_XKY_PCI_GetFunctionFromDirection)
}

PUBLIC NAKED dword XKY_PCI_ReadDword(IN PCI _device, IN dword _offset)
{
	CALL2(IDX_XKY_PCI_ReadDword)
}

PUBLIC NAKED void XKY_PCI_WriteDword(IN PCI _device, IN dword _offset, IN dword _data)
{
	CALL3(IDX_XKY_PCI_WriteDword)
}

//CPU
PUBLIC NAKED void XKY_CPU_FillExecution(IN EXECUTION* _execution, IN ADDRESS_SPACE _pdbr, IN VIRTUAL _code, IN VIRTUAL _stack, IN bool _user_mode)
{
	CALL5(IDX_XKY_CPU_FillExecution)
}

PUBLIC NAKED XID XKY_CPU_Alloc(IN XID _desired, IN XID _existing)
{
	CALL2(IDX_XKY_CPU_Alloc)
}

PUBLIC NAKED XID XKY_CPU_AllocCode(IN XID _desired, IN ADDRESS_SPACE _address_space, IN VIRTUAL _code, IN VIRTUAL _stack)
{
	CALL4(IDX_XKY_CPU_AllocCode)
}

PUBLIC NAKED void XKY_CPU_Free(IN XID _xid)
{
	CALL1(IDX_XKY_CPU_Free)
}

PUBLIC NAKED void XKY_CPU_RegisterCallback(IN ADDRESS_SPACE _address_space, IN fCpuTimerCallback _callback)
{
	CALL2(IDX_XKY_CPU_RegisterCallback)
}

PUBLIC NAKED XID XKY_CPU_GetCurrent()
{
	CALL0(IDX_XKY_CPU_GetCurrent)
}

//RTC
PUBLIC NAKED byte XKY_RTC_Seconds()
{
	CALL0(IDX_XKY_RTC_Seconds)
}

PUBLIC NAKED byte XKY_RTC_Minutes()
{
	CALL0(IDX_XKY_RTC_Minutes)
}

PUBLIC NAKED byte XKY_RTC_Hour()
{
	CALL0(IDX_XKY_RTC_Hour)
}

PUBLIC NAKED DaysOfWeek XKY_RTC_DayOfWeek()
{
	CALL0(IDX_XKY_RTC_DayOfWeek)
}

PUBLIC NAKED dword XKY_RTC_DayOfMonth()
{
	CALL0(IDX_XKY_RTC_DayOfMonth)
}

PUBLIC NAKED byte XKY_RTC_Month()
{
	CALL0(IDX_XKY_RTC_Month)
}

PUBLIC NAKED dword XKY_RTC_Year()
{
	CALL0(IDX_XKY_RTC_Year)
}

//TMR
PUBLIC NAKED dword XKY_TMR_GetTicks()
{
	CALL0(IDX_XKY_TMR_GetTicks)
}

//LDR
PUBLIC NAKED VIRTUAL XKY_LDR_GetProcedureAddress(IN VIRTUAL _module, IN string* function_name)
{
	CALL2(IDX_XKY_LDR_GetProcedureAddress)
}

PUBLIC NAKED bool XKY_LDR_LoadUserModule(IN string* _module, IN ADDRESS_SPACE _pbr, IN VIRTUAL _base)
{
	CALL3(IDX_XKY_LDR_LoadUserModule)
}

PUBLIC NAKED bool XKY_LDR_LoadKernelModule(IN string* _module)
{
	CALL1(IDX_XKY_LDR_LoadKernelModule)
}

PUBLIC NAKED bool XKY_LDR_LoadFile(IN string* _file, IN ADDRESS_SPACE _pbr, IN VIRTUAL _base)
{
	CALL3(IDX_XKY_LDR_LoadFile)
}

PUBLIC NAKED dword XKY_LDR_FileSize(IN string* _file)
{
	CALL1(IDX_XKY_LDR_FileSize)
}

//OS
PUBLIC NAKED bool XKY_OS_Start(IN string* _module)
{
	CALL1(IDX_XKY_OS_Start)
}

PUBLIC NAKED void XKY_OS_Finish()
{
	CALL0(IDX_XKY_OS_Finish)
}

//EXCEPTION
PUBLIC NAKED bool XKY_EXCEPTION_SetHandler(IN byte _exception, IN ADDRESS_SPACE _pdbr, IN fInterruptHandler _handler)
{
	CALL3(IDX_XKY_EXCEPTION_SetHandler)
}

PUBLIC NAKED void XKY_EXCEPTION_UnsetHandler(IN byte _exception)
{
	CALL1(IDX_XKY_EXCEPTION_UnsetHandler)
}

//DEBUG
PUBLIC NAKED void XKY_DEBUG_Message(IN string* _message, IN dword _color)
{
	CALL2(IDX_XKY_DEBUG_Message)
}

PUBLIC NAKED void XKY_DEBUG_Data(IN string* _message, IN dword _data, IN dword _color)
{
	CALL3(IDX_XKY_DEBUG_Data)
}

//=================================EXPORTS====================================//
#pragma data_seg(".exports")
//============================================================================//
//Memory
EXPORT(XKY_ADDRESS_SPACE_Alloc);
EXPORT(XKY_ADDRESS_SPACE_Free);
EXPORT(XKY_ADDRESS_SPACE_GetCurrent);
EXPORT(XKY_ADDRESS_SPACE_SwitchTo);
EXPORT(XKY_PAGE_Alloc);
EXPORT(XKY_PAGE_Free);
EXPORT(XKY_PAGE_Share);

//Disk
EXPORT(XKY_DISK_Alloc);
EXPORT(XKY_DISK_Free);
EXPORT(XKY_DISK_Read);
EXPORT(XKY_DISK_Write);

//Windows
EXPORT(XKY_WINDOW_Alloc);
EXPORT(XKY_WINDOW_Free);
EXPORT(XKY_WINDOW_GetHeight);
EXPORT(XKY_WINDOW_GetWidth);
EXPORT(XKY_WINDOW_GetPixel);
EXPORT(XKY_WINDOW_SetPixel);
EXPORT(XKY_WINDOW_PrintText);
EXPORT(XKY_WINDOW_RegisterKeyboard);
EXPORT(XKY_WINDOW_RegisterMouse);
EXPORT(XKY_POINTER_GetArrow);
EXPORT(XKY_POINTER_GetClock);
EXPORT(XKY_WINDOW_SetPointer);
EXPORT(XKY_WINDOW_SetPointerColor);

//PCI
EXPORT(XKY_PCI_Alloc);
EXPORT(XKY_PCI_Free);
EXPORT(XKY_PCI_GetNumberOfDevices);
EXPORT(XKY_PCI_GetDevice);
EXPORT(XKY_PCI_GetBusFromDirection);
EXPORT(XKY_PCI_GetDeviceFromDirection);
EXPORT(XKY_PCI_GetFunctionFromDirection);
EXPORT(XKY_PCI_ReadDword);
EXPORT(XKY_PCI_WriteDword);

//CPU
EXPORT(XKY_CPU_FillExecution);
EXPORT(XKY_CPU_Alloc);
EXPORT(XKY_CPU_AllocCode);
EXPORT(XKY_CPU_Free);
EXPORT(XKY_CPU_RegisterCallback);
EXPORT(XKY_CPU_GetCurrent);

//RTC
EXPORT(XKY_RTC_Seconds);
EXPORT(XKY_RTC_Minutes);
EXPORT(XKY_RTC_Hour);
EXPORT(XKY_RTC_DayOfWeek);
EXPORT(XKY_RTC_DayOfMonth);
EXPORT(XKY_RTC_Month);
EXPORT(XKY_RTC_Year);

//RTC
EXPORT(XKY_TMR_GetTicks);

//LDR
EXPORT(XKY_LDR_GetProcedureAddress);
EXPORT(XKY_LDR_LoadUserModule);
EXPORT(XKY_LDR_LoadKernelModule);
EXPORT(XKY_LDR_LoadFile);
EXPORT(XKY_LDR_FileSize);

//OS
EXPORT(XKY_OS_Start);
EXPORT(XKY_OS_Finish);

//EXCEPTION
EXPORT(XKY_EXCEPTION_SetHandler);
EXPORT(XKY_EXCEPTION_UnsetHandler);

//DEBUG
EXPORT(XKY_DEBUG_Message);
EXPORT(XKY_DEBUG_Data);


//=================================MODULE=====================================//
#pragma data_seg(".module")
//============================================================================//
	MODULE(IMAGE_MODE_SYSTEM, IMAGE_KIND_MODULE, IMAGE_VERSION(1,0,0,0), 0, API_EntryPoint, 0);
