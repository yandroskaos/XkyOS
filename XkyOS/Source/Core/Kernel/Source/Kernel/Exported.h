#ifndef __EXPORTED_H__
#define __EXPORTED_H__

	#include "Types.h"
	#include "AddressSpace.h"
	#include "DiskRange.h"
	#include "Windows.h"
	#include "PCI.h"
	#include "Processor.h"
	#include "RTC.h"
	#include "Timer.h"
	#include "Environment.h"
	#include "Interrupts.h"
	#include "RTL.h"
	#include "Debug.h"

	//Memory
	ADDRESS_SPACE	XKY_ADDRESS_SPACE_Alloc			();
	bool			XKY_ADDRESS_SPACE_Free			(IN ADDRESS_SPACE _address_space);
	ADDRESS_SPACE	XKY_ADDRESS_SPACE_GetCurrent	();
	bool			XKY_ADDRESS_SPACE_SwitchTo		(IN ADDRESS_SPACE _address_space);

	VIRTUAL			XKY_PAGE_Alloc	(IN ADDRESS_SPACE _address_space, IN VIRTUAL _address, IN dword _number_of_pages);
	void			XKY_PAGE_Free	(IN ADDRESS_SPACE _address_space, IN VIRTUAL _address, IN dword number_of_pages);
	bool			XKY_PAGE_Share	(IN ADDRESS_SPACE _pdbr_origin, IN VIRTUAL _origin, IN ADDRESS_SPACE _pdbr_destiny, IN VIRTUAL _destiny, IN dword number_of_pages);

	//Disk
	LBA		XKY_DISK_Alloc	(IN LBA _sector, IN dword _number_of_sectors);
	void	XKY_DISK_Free	(IN LBA _sector, IN dword _number_of_sectors);
	bool	XKY_DISK_Read	(IN LBA _sector, IN VIRTUAL _memory, IN dword _number_of_sectors);
	bool	XKY_DISK_Write	(IN LBA _sector, IN VIRTUAL _memory, IN dword _number_of_sectors);

	//Windows
	WINDOW	XKY_WINDOW_Alloc	();
	void	XKY_WINDOW_Free		(IN WINDOW _window);
	dword	XKY_WINDOW_GetHeight		(IN WINDOW _window);
	dword	XKY_WINDOW_GetWidth			(IN WINDOW _window);
	ARGB	XKY_WINDOW_GetPixel			(IN WINDOW _window, IN dword _x, IN dword _y);
	void	XKY_WINDOW_SetPixel			(IN WINDOW _window, IN dword _x, IN dword _y, IN ARGB _color);
	void	XKY_WINDOW_PrintText		(IN WINDOW _window, IN dword _x, IN dword _y, IN ARGB _color, IN string* _text);
	void	XKY_WINDOW_RegisterKeyboard	(IN WINDOW _window, IN ADDRESS_SPACE _pdbr, IN fWindowKeyboardCallback _callback);
	void	XKY_WINDOW_RegisterMouse	(IN WINDOW _window, IN ADDRESS_SPACE _pdbr, IN fWindowMouseCallback _callback);
	POINTER	XKY_POINTER_GetArrow		();
	POINTER	XKY_POINTER_GetClock		();
	void	XKY_WINDOW_SetPointer		(IN WINDOW _window, IN POINTER _pointer);
	void	XKY_WINDOW_SetPointerColor	(IN WINDOW _window, IN ARGB _color);

	//PCI
	PCI		XKY_PCI_Alloc	(IN PCI _device);
	void	XKY_PCI_Free	(IN PCI _device);
	dword	XKY_PCI_GetNumberOfDevices	();
	PCI		XKY_PCI_GetDevice			(IN dword _index);
	byte	XKY_PCI_GetBusFromDirection			(IN PCI _device);
	byte	XKY_PCI_GetDeviceFromDirection		(IN PCI _device);
	byte	XKY_PCI_GetFunctionFromDirection	(IN PCI _device);
	dword	XKY_PCI_ReadDword	(IN PCI _device, IN dword _offset);
	void	XKY_PCI_WriteDword	(IN PCI _device, IN dword _offset, IN dword _data);

	//CPU
	void	XKY_CPU_FillExecution	(IN EXECUTION* _execution, IN ADDRESS_SPACE _pdbr, IN VIRTUAL _code, IN VIRTUAL _stack, IN bool _user_mode);
	XID		XKY_CPU_Alloc			(IN XID _desired, IN XID _existing);
	XID		XKY_CPU_AllocCode		(IN XID _desired, IN ADDRESS_SPACE _pdbr, IN VIRTUAL _code, IN VIRTUAL _stack);
	void	XKY_CPU_Free			(IN XID _xid);
	void	XKY_CPU_RegisterCallback(IN ADDRESS_SPACE _pdbr, IN fCpuTimerCallback _callback);
	XID		XKY_CPU_GetCurrent		();

	//RTC
	byte		XKY_RTC_Seconds		();
	byte		XKY_RTC_Minutes		();
	byte		XKY_RTC_Hour		();
	DaysOfWeek	XKY_RTC_DayOfWeek	();
	byte		XKY_RTC_DayOfMonth	();
	byte		XKY_RTC_Month		();
	dword		XKY_RTC_Year		();

	//Timer
	dword	XKY_TMR_GetTicks	();

	//LDR
	VIRTUAL	XKY_LDR_GetProcedureAddress	(IN VIRTUAL _module, IN string* _function_name);
	bool	XKY_LDR_LoadUserModule		(IN string* _module, IN ADDRESS_SPACE _pdbr, IN VIRTUAL _base);
	bool	XKY_LDR_LoadKernelModule	(IN string* _module);
	bool	XKY_LDR_LoadFile			(IN string* _file, IN ADDRESS_SPACE _pdbr, IN VIRTUAL _base);
	dword	XKY_LDR_FileSize			(IN string* _file);

	//OS
	bool	XKY_OS_Start	(IN string* _module);
	void	XKY_OS_Finish	();

	//EXCEPTION
	bool XKY_EXCEPTION_SetHandler	(IN byte _exception, IN ADDRESS_SPACE _pdbr, IN fInterruptHandler _handler);
	void XKY_EXCEPTION_UnsetHandler	(IN byte _exception);
	bool XKY_INTERRUPT_SetHandler	(IN byte _interrupt, IN fInterruptHandler _handler);
	void XKY_INTERRUPT_UnsetHandler	(IN byte _interrupt, IN fInterruptHandler _handler);

	//DEBUG
	void XKY_DEBUG_Message	(IN string* _message, IN dword _color);
	void XKY_DEBUG_Data		(IN string* _message, IN dword _data, IN dword _color);

#endif //__EXPORTED_H__
