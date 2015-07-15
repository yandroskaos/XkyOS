/******************************************************************************/
/**
* @file		API.h
* @brief	XkyOS Services user-mode stub module
* Module loaded in user-mode environment addresses to allow to communicate with the kernel.
* 
* @date		20/03/2008
* @author	Pablo Bravo
*/
/******************************************************************************/
#ifndef __API_H__
#define __API_H__

//Memory
typedef dword ADDRESS_SPACE;
typedef dword VIRTUAL;
#define PAGE_SIZE 4096

typedef ADDRESS_SPACE	(*fXKY_ADDRESS_SPACE_Alloc)			();
typedef bool			(*fXKY_ADDRESS_SPACE_Free)			(IN ADDRESS_SPACE _address_space);
typedef ADDRESS_SPACE	(*fXKY_ADDRESS_SPACE_GetCurrent)	();
typedef bool			(*fXKY_ADDRESS_SPACE_SwitchTo)		(IN ADDRESS_SPACE _address_space);
IMPORT(XKY_ADDRESS_SPACE_Alloc);
IMPORT(XKY_ADDRESS_SPACE_Free);
IMPORT(XKY_ADDRESS_SPACE_GetCurrent);
IMPORT(XKY_ADDRESS_SPACE_SwitchTo);

typedef VIRTUAL	(*fXKY_PAGE_Alloc)	(IN ADDRESS_SPACE _address_space, IN VIRTUAL _address, IN dword _number_of_pages);
typedef void	(*fXKY_PAGE_Free)	(IN ADDRESS_SPACE _address_space, IN VIRTUAL _address, IN dword number_of_pages);
typedef bool	(*fXKY_PAGE_Share)	(IN ADDRESS_SPACE _pdbr_origin, IN VIRTUAL _origin, IN ADDRESS_SPACE _pdbr_destiny, IN VIRTUAL _destiny, IN dword number_of_pages);
IMPORT(XKY_PAGE_Alloc);
IMPORT(XKY_PAGE_Free);
IMPORT(XKY_PAGE_Share);

//Disk
typedef dword LBA;
#define SECTOR_SIZE 512

typedef LBA		(*fXKY_DISK_Alloc)	(IN LBA _sector, IN dword _number_of_sectors);
typedef void	(*fXKY_DISK_Free)	(IN LBA _sector, IN dword _number_of_sectors);

typedef bool	(*fXKY_DISK_Read)	(IN LBA _sector, IN VIRTUAL _memory, IN dword _number_of_sectors);
typedef bool	(*fXKY_DISK_Write)	(IN LBA _sector, IN VIRTUAL _memory, IN dword _number_of_sectors);
IMPORT(XKY_DISK_Alloc);
IMPORT(XKY_DISK_Free);
IMPORT(XKY_DISK_Read);
IMPORT(XKY_DISK_Write);

//Windows
typedef dword WINDOW;
typedef dword ARGB;
typedef dword POINTER;
typedef void (*fWindowMouseCallback)	(IN dword _x, IN dword _y, IN bool _left_clicked, IN bool _right_clicked);
typedef void (*fWindowKeyboardCallback)	(IN dword _scan_code);

#define SRGB(R,G,B) ((ARGB)((0x00<<24)|((R)<<16)|((G)<<8)|((B)))) //ARGB SolidRGB(byte R, byte G, byte B);
#define TRGB(R,G,B) ((ARGB)((0xFF<<24)|((R)<<16)|((G)<<8)|((B)))) //ARGB TransparentRGB(byte R, byte G, byte B);

typedef WINDOW	(*fXKY_WINDOW_Alloc)	();
typedef void	(*fXKY_WINDOW_Free)		(IN WINDOW _window);

typedef dword	(*fXKY_WINDOW_GetHeight)		(IN WINDOW _window);
typedef dword	(*fXKY_WINDOW_GetWidth)			(IN WINDOW _window);
typedef ARGB	(*fXKY_WINDOW_GetPixel)			(IN WINDOW _window, IN dword _x, IN dword _y);
typedef void	(*fXKY_WINDOW_SetPixel)			(IN WINDOW _window, IN dword _x, IN dword _y, IN ARGB _color);
typedef void	(*fXKY_WINDOW_PrintText)		(IN WINDOW _window, IN dword _x, IN dword _y, IN ARGB _color, IN string* _text);
typedef void	(*fXKY_WINDOW_RegisterKeyboard)	(IN WINDOW _window, IN ADDRESS_SPACE _pdbr, IN fWindowKeyboardCallback _callback);
typedef void	(*fXKY_WINDOW_RegisterMouse)	(IN WINDOW _window, IN ADDRESS_SPACE _pdbr, IN fWindowMouseCallback _callback);
typedef POINTER	(*fXKY_POINTER_GetArrow)		();
typedef POINTER	(*fXKY_POINTER_GetClock)		();
typedef void	(*fXKY_WINDOW_SetPointer)		(IN WINDOW _window, IN POINTER _pointer);
typedef void	(*fXKY_WINDOW_SetPointerColor)	(IN WINDOW _window, IN ARGB _color);
IMPORT(XKY_WINDOW_Alloc);
IMPORT(XKY_WINDOW_Free);
IMPORT(XKY_WINDOW_GetHeight);
IMPORT(XKY_WINDOW_GetWidth);
IMPORT(XKY_WINDOW_GetPixel);
IMPORT(XKY_WINDOW_SetPixel);
IMPORT(XKY_WINDOW_PrintText);
IMPORT(XKY_WINDOW_RegisterKeyboard);
IMPORT(XKY_WINDOW_RegisterMouse);
IMPORT(XKY_POINTER_GetArrow);
IMPORT(XKY_POINTER_GetClock);
IMPORT(XKY_WINDOW_SetPointer);
IMPORT(XKY_WINDOW_SetPointerColor);

//PCI
typedef dword PCI;
typedef PCI		(*fXKY_PCI_Alloc)	(IN PCI _device);
typedef void	(*fXKY_PCI_Free)	(IN PCI _device);

typedef dword	(*fXKY_PCI_GetNumberOfDevices)	();
typedef PCI		(*fXKY_PCI_GetDevice)			(IN dword _index);
typedef byte	(*fXKY_PCI_GetBusFromDirection)			(IN PCI _device);
typedef byte	(*fXKY_PCI_GetDeviceFromDirection)		(IN PCI _device);
typedef byte	(*fXKY_PCI_GetFunctionFromDirection)	(IN PCI _device);
typedef dword	(*fXKY_PCI_ReadDword)	(IN PCI _device, IN dword _offset);
typedef void	(*fXKY_PCI_WriteDword)	(IN PCI _device, IN dword _offset, IN dword _data);
IMPORT(XKY_PCI_Alloc);
IMPORT(XKY_PCI_Free);
IMPORT(XKY_PCI_GetNumberOfDevices);
IMPORT(XKY_PCI_GetDevice);
IMPORT(XKY_PCI_GetBusFromDirection);
IMPORT(XKY_PCI_GetDeviceFromDirection);
IMPORT(XKY_PCI_GetFunctionFromDirection);
IMPORT(XKY_PCI_ReadDword);
IMPORT(XKY_PCI_WriteDword);

//CPU
typedef dword XID;
#define XID_ANY 0xFFFFFFFF
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

typedef void	(*fXKY_CPU_FillExecution)	(IN EXECUTION* _execution, IN ADDRESS_SPACE _pdbr, IN VIRTUAL _code, IN VIRTUAL _stack, IN bool _user_mode);
typedef XID		(*fXKY_CPU_Alloc)			(IN XID _desired, IN XID _existing);
typedef XID		(*fXKY_CPU_AllocCode)		(IN XID _desired, IN ADDRESS_SPACE _pdbr, IN VIRTUAL _code, IN VIRTUAL _stack);
typedef void	(*fXKY_CPU_Free)			(IN XID _xid);
typedef void	(*fXKY_CPU_RegisterCallback)(IN ADDRESS_SPACE _pdbr, IN fCpuTimerCallback _callback);
typedef XID		(*fXKY_CPU_GetCurrent)		();
IMPORT(XKY_CPU_FillExecution);
IMPORT(XKY_CPU_Alloc);
IMPORT(XKY_CPU_AllocCode);
IMPORT(XKY_CPU_Free);
IMPORT(XKY_CPU_RegisterCallback);
IMPORT(XKY_CPU_GetCurrent);

//RTC
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

typedef byte		(*fXKY_RTC_Seconds)		();
typedef byte		(*fXKY_RTC_Minutes)		();
typedef byte		(*fXKY_RTC_Hour)		();
typedef DaysOfWeek	(*fXKY_RTC_DayOfWeek)	();
typedef byte		(*fXKY_RTC_DayOfMonth)	();
typedef byte		(*fXKY_RTC_Month)		();
typedef dword		(*fXKY_RTC_Year)		();
IMPORT(XKY_RTC_Seconds);
IMPORT(XKY_RTC_Minutes);
IMPORT(XKY_RTC_Hour);
IMPORT(XKY_RTC_DayOfWeek);
IMPORT(XKY_RTC_DayOfMonth);
IMPORT(XKY_RTC_Month);
IMPORT(XKY_RTC_Year);

//TMR
typedef dword (*fXKY_TMR_GetTicks)	();
IMPORT(XKY_TMR_GetTicks);

//LDR
typedef VIRTUAL	(*fXKY_LDR_GetProcedureAddress)	(IN VIRTUAL _module, IN string* _function_name);
typedef bool	(*fXKY_LDR_LoadUserModule)		(IN string* _module,  IN ADDRESS_SPACE _pbr, IN VIRTUAL _base);
typedef bool	(*fXKY_LDR_LoadKernelModule)	(IN string* _module);
typedef bool	(*fXKY_LDR_LoadFile)			(IN string* _file,  IN ADDRESS_SPACE _pbr, IN VIRTUAL _base);
typedef dword	(*fXKY_LDR_FileSize)			(IN string* _file);

IMPORT(XKY_LDR_GetProcedureAddress);
IMPORT(XKY_LDR_LoadUserModule);
IMPORT(XKY_LDR_LoadKernelModule);
IMPORT(XKY_LDR_LoadFile);
IMPORT(XKY_LDR_FileSize);

//OS
typedef bool	(*fXKY_OS_Start)	(IN string* _module);
typedef void	(*fXKY_OS_Finish)	();
IMPORT(XKY_OS_Start);
IMPORT(XKY_OS_Finish);

//EXCEPTION
struct INTERRUPT_FRAME
{
	//Selectors
	dword gs;
	dword fs;
	dword es;
	dword ds;
	//Generic-purpose registers
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

typedef bool (*fXKY_EXCEPTION_SetHandler)	(IN byte _exception, IN ADDRESS_SPACE _pdbr, IN fInterruptHandler _handler);
typedef void (*fXKY_EXCEPTION_UnsetHandler)	(IN byte _exception);

IMPORT(XKY_EXCEPTION_SetHandler);
IMPORT(XKY_EXCEPTION_UnsetHandler);

//DEBUG
typedef void (*fXKY_DEBUG_Message)	(IN string* _message, IN dword _color);
typedef void (*fXKY_DEBUG_Data)		(IN string* _message, IN dword _data, IN dword _color);

IMPORT(XKY_DEBUG_Message);
IMPORT(XKY_DEBUG_Data);

#endif //__API_H__
