#include "Exported.h"

//==================================DATA======================================//
#pragma data_seg(".data")
//============================================================================//
BOUNDED_STRING(kernel_module_name, 255, 0);
BOUNDED_STRING(dynamic_module_name, 255, 0);

//==================================CODE======================================//
#pragma code_seg(".code")
//============================================================================//

//Memory
/**
* @brief Allocates a new address space.
* @return The newly created address space, 0 otherwise.
*/
PUBLIC ADDRESS_SPACE XKY_ADDRESS_SPACE_Alloc()
{
	ADDRESS_SPACE current = ADDRESS_SPACE_GetCurrent();
	ADDRESS_SPACE_ResetToKernelSpace();

	ADDRESS_SPACE address_space = ADDRESS_SPACE_Create();
	if(address_space)
	{
		if(ENVIRONMENT_AllocPDBR(ENVIRONMENT_GetCurrent(), address_space))
		{
			ADDRESS_SPACE_SwitchTo(current);
			return address_space;
		}
		ADDRESS_SPACE_Release(address_space);
	}
	ADDRESS_SPACE_SwitchTo(current);
	return 0;
}

/**
* @brief Frees an address space.
* @param _address_space [in] The address space to release.
* @return True if released, false otherwise.
*/
PUBLIC bool XKY_ADDRESS_SPACE_Free(IN ADDRESS_SPACE _address_space)
{
	if(ADDRESS_SPACE_GetCurrent() == _address_space)
		return false;

	ADDRESS_SPACE current = ADDRESS_SPACE_GetCurrent();
	ADDRESS_SPACE_ResetToKernelSpace();

	if(ENVIRONMENT_FreePDBR(ENVIRONMENT_GetCurrent(), _address_space))
	{
		ADDRESS_SPACE_Release(_address_space);
	}

	ADDRESS_SPACE_SwitchTo(current);
	return true;
}

/**
* @brief Consult current address space.
* @return Cuurent address space.
*/
PUBLIC ADDRESS_SPACE XKY_ADDRESS_SPACE_GetCurrent()
{
	return ADDRESS_SPACE_GetCurrent();
}

PUBLIC bool XKY_ADDRESS_SPACE_SwitchTo(IN ADDRESS_SPACE _address_space)
{
	if(ADDRESS_SPACE_GetCurrent() != _address_space)
	{
		if(ENVIRONMENT_OwnsPDBR(ENVIRONMENT_GetCurrent(), _address_space))
		{
			ADDRESS_SPACE_SwitchTo(_address_space);
			return true;
		}
	}
	return false;
}

/**
* @brief Allocates a number of pages.
* @param _address_space [in] The address space where the page will be mapped.
* @param _address [in] The virtual address where the page will be mapped.
* @param _number_of_pages [in] The number of pages to allocate.
* @return The virtual address reserved, 0 otherwise.
*/
PUBLIC VIRTUAL XKY_PAGE_Alloc(IN ADDRESS_SPACE _address_space, IN VIRTUAL _address, IN dword _number_of_pages)
{
	ADDRESS_SPACE current = ADDRESS_SPACE_GetCurrent();
	ADDRESS_SPACE_ResetToKernelSpace();

	//Do page allocation
	PHYSICAL pages = MEM_AllocPages(_number_of_pages, UserMode);
	if(!pages)
	{
		ADDRESS_SPACE_SwitchTo(current);
		return false;
	}

	bool success = ADDRESS_SPACE_Map(_address_space, pages, _address, _number_of_pages, UserMode, ReadWrite, true);
	if(!success)
	{
		MEM_ReleasePages(pages, _number_of_pages);
	}

	ADDRESS_SPACE_SwitchTo(current);
	return success?_address:0;
}

/**
* @brief Frees a number of pages.
* @param _address_space [in] The address space where the page will be mapped.
* @param _address [in] The virtual address where the page will be mapped.
* @param _number_of_pages [in] The number of pages to allocate.
*/
PUBLIC void XKY_PAGE_Free(IN ADDRESS_SPACE _address_space, IN VIRTUAL _address, IN dword _number_of_pages)
{
	ADDRESS_SPACE current = ADDRESS_SPACE_GetCurrent();
	ADDRESS_SPACE_ResetToKernelSpace();

	ADDRESS_SPACE_Unmap(_address_space, _address, _number_of_pages);

	ADDRESS_SPACE_SwitchTo(current);
}

/**
* @brief Shares a number of pages between address spaces.
* @param _pdbr_origin [in] The address space origin where the pages are already mapped.
* @param _origin [in] The virtual address from where we want to start the sharing.
* @param _pdbr_destiny [in] The address space destiny where the pages will be mapped.
* @param _destiny [in] The virtual address where the pages will be mapped in _pdbr_destiny.
* @param _number_of_pages [in] Number of pages to share.
* @return True if all ok, false otherwise.
*/
PUBLIC bool XKY_PAGE_Share(IN ADDRESS_SPACE _pdbr_origin, IN VIRTUAL _origin, IN ADDRESS_SPACE _pdbr_destiny, IN VIRTUAL _destiny, IN dword _number_of_pages)
{
	ADDRESS_SPACE current = ADDRESS_SPACE_GetCurrent();
	ADDRESS_SPACE_ResetToKernelSpace();

	//Check _pdbr_origin has all pages at _origin mapped
	if(!ADDRESS_SPACE_IsMapped(_pdbr_origin, _origin, _number_of_pages))
	{
		ADDRESS_SPACE_SwitchTo(current);
		return false;
	}

	//Check _pdbr_destiny has free room at _destiny for all pages
	for(dword i = 0; i < _number_of_pages; i++)
	{
		if(ADDRESS_SPACE_IsMapped(_pdbr_destiny, _destiny + i*PAGE_SIZE, 1))
		{
			ADDRESS_SPACE_SwitchTo(current);
			return false;
		}
	}

	//Ok, map
	for(dword i = 0; i < _number_of_pages; i++)
	{
		PHYSICAL physical = VIRTUAL_GetPhysical(_pdbr_origin, _origin + i*PAGE_SIZE);
		if(!ADDRESS_SPACE_Map(_pdbr_destiny, physical, _destiny + i*PAGE_SIZE, 1, UserMode, ReadWrite, false))
		{
			ADDRESS_SPACE_Unmap(_pdbr_destiny, _destiny, i);
			return false;
		}
	}

	ADDRESS_SPACE_SwitchTo(current);
	return true;
}

//Disk
/**
* @brief Allocates a number of disk sectors.
* @param _sector [in] The sector we want to allocate.
* @param _number_of_sectors [in] Number of sectors to allocate.
* @return The address asked for if successful, false otherwise.
*/
PUBLIC LBA XKY_DISK_Alloc(IN LBA _sector, IN dword _number_of_sectors)
{
	if(_sector >= HD_Size())
		return 0;

	//First 4MB are for kernel
	#define KERNEL_DISK_MEGAS	4
	if((HD_BootDrive() == HD_DRIVE) && (_sector < KERNEL_DISK_MEGAS*1024*1024/SECTOR_SIZE))
		return 0;

	//Check sectors are not used
	DISK_RANGE range;
	DISK_RANGE_Fill(range, _sector, _number_of_sectors);

	for(ENVIRONMENT* e = ENVIRONMENT_First(); e; e = ENVIRONMENT_Next(e))
	{
		if(ENVIRONMENT_OwnsDISK(e, range))
		{
			return 0;
		}
	}

	//Ok, they're empty, so add to current
	if(!ENVIRONMENT_AllocDISK(ENVIRONMENT_GetCurrent(), range))
		return 0;

	//Return the address of the allocated sector
	return _sector;
}

/**
* @brief Releases a number of disk sectors.
* @param _sector [in] The sector we want to release.
* @param _number_of_sectors [in] Number of sectors to free.
*/
PUBLIC void XKY_DISK_Free(IN LBA _sector, IN dword _number_of_sectors)
{
	ENVIRONMENT_FreeDISK(ENVIRONMENT_GetCurrent(), _sector);
}

/**
* @brief Reads a number of disk sectors.
* @param _sector [in] The sector we want to read.
* @param _memory [in] The buffer where the data will be stored.
* @param _number_of_sectors [in] Number of sectors to read.
* @return True if successful, false otherwise.
*/
PUBLIC bool XKY_DISK_Read(IN LBA _sector, IN VIRTUAL _memory, IN dword _number_of_sectors)
{
	DISK_RANGE range;
	DISK_RANGE_Fill(range, _sector, _number_of_sectors);

	//Check if lba's is owned by environment
	if(!ENVIRONMENT_OwnsDISK(ENVIRONMENT_GetCurrent(), range))
	{
		return false;
	}
	
	//Do read
	return HD_ReadSectors(0, _sector, _number_of_sectors, _memory) == _number_of_sectors;
}

/**
* @brief Writes a number of disk sectors.
* @param _sector [in] The sector we want to write to.
* @param _memory [in] The buffer where the data will be read from.
* @param _number_of_sectors [in] Number of sectors to write.
* @return True if successful, false otherwise.
*/
PUBLIC bool XKY_DISK_Write(IN LBA _sector, IN VIRTUAL _memory, IN dword _number_of_sectors)
{
	DISK_RANGE range;
	DISK_RANGE_Fill(range, _sector, _number_of_sectors);

	//Check if lba's is owned by environment
	if(!ENVIRONMENT_OwnsDISK(ENVIRONMENT_GetCurrent(), range))
	{
		return false;
	}

	//Do write
	return HD_WriteSectors(0, _sector, _number_of_sectors, _memory) == _number_of_sectors;
}

//Windows
/**
* @brief Allocates a window.
* @return The window allocated if successful, 0 otherwise.
*/
PUBLIC WINDOW XKY_WINDOW_Alloc()
{
	//Get a window
	WINDOW window = WINDOW_GetWindow();
	if(window)
	{
		if(ENVIRONMENT_AllocWINDOW(ENVIRONMENT_GetCurrent(), window))
			return window;
		WINDOW_ReleaseWindow(window);
	}
	return 0;
}

/**
* @brief Releases a window.
* @param _window [in] The window to release.
*/
PUBLIC void XKY_WINDOW_Free(IN WINDOW _window)
{
	if(_window && ENVIRONMENT_OwnsWINDOW(ENVIRONMENT_GetCurrent(), _window))
	{
		ENVIRONMENT_FreeWINDOW(ENVIRONMENT_GetCurrent(), _window);
		WINDOW_ReleaseWindow(_window);
	}
}

/**
* @brief Get the height of a window.
* @param _window [in] The window.
* @return The height of the window if successful, 0 otherwise.
*/
PUBLIC dword XKY_WINDOW_GetHeight(IN WINDOW _window)
{
	if(_window && ENVIRONMENT_OwnsWINDOW(ENVIRONMENT_GetCurrent(), _window))
	{
		return WINDOW_GetHeight(_window);
	}
	return 0;
}

/**
* @brief Get the width of a window.
* @param _window [in] The window.
* @return The width of the window if successful, 0 otherwise.
*/
PUBLIC dword XKY_WINDOW_GetWidth(IN WINDOW _window)
{
	if(_window && ENVIRONMENT_OwnsWINDOW(ENVIRONMENT_GetCurrent(), _window))
	{
		return WINDOW_GetWidth(_window);
	}
	return 0;
}

/**
* @brief Get a pixel from a window.
* @param _window [in] The window.
* @param _x [in] The x coordinate of the pixel.
* @param _y [in] The y coordinate of the pixel.
* @return The ARGB value of the pixel if successful, 0 otherwise.
*/
PUBLIC ARGB XKY_WINDOW_GetPixel(IN WINDOW _window, IN dword _x, IN dword _y)
{
	if(_window && ENVIRONMENT_OwnsWINDOW(ENVIRONMENT_GetCurrent(), _window))
	{
		return WINDOW_GetPixel(_window, _x, _y);
	}
	return 0;
}

/**
* @brief Sets a pixel in a window.
* @param _window [in] The window.
* @param _x [in] The x coordinate of the pixel.
* @param _y [in] The y coordinate of the pixel.
* @param _color [in] The ARGB value of the pixel.
*/
PUBLIC void XKY_WINDOW_SetPixel(IN WINDOW _window, IN dword _x, IN dword _y, IN ARGB _color)
{
	if(_window && ENVIRONMENT_OwnsWINDOW(ENVIRONMENT_GetCurrent(), _window))
	{
		WINDOW_SetPixel(_window, _x, _y, _color);
	}
}

/**
* @brief Prints a string in a window.
* @param _window [in] The window.
* @param _x [in] The x coordinate of the text.
* @param _y [in] The y coordinate of the text.
* @param _color [in] The color of the text.
* @param _text [in] The string to print.
*/
PUBLIC void XKY_WINDOW_PrintText(IN WINDOW _window, IN dword _x, IN dword _y, IN ARGB _color, IN string* _text)
{
	if(_window && ENVIRONMENT_OwnsWINDOW(ENVIRONMENT_GetCurrent(), _window))
	{
		WINDOW_PrintText(_window, _x, _y, _color, _text);
	}
}

/**
* @brief Register a keyboard callback for a window.
* @param _window [in] The window.
* @param _pdbr [in] The address space where the function callback is valid.
* @param _callback [in] The function callback.
*/
PUBLIC void XKY_WINDOW_RegisterKeyboard(IN WINDOW _window, IN ADDRESS_SPACE _pdbr, IN fWindowKeyboardCallback _callback)
{
	if(_window && ENVIRONMENT_OwnsWINDOW(ENVIRONMENT_GetCurrent(), _window) && ENVIRONMENT_OwnsPDBR(ENVIRONMENT_GetCurrent(), _pdbr))
	{
		WINDOW_RegisterKeyboard(_window, _pdbr, _callback);
	}
}

/**
* @brief Register a mouse callback for a window.
* @param _window [in] The window.
* @param _pdbr [in] The address space where the function callback is valid.
* @param _callback [in] The function callback.
*/
PUBLIC void XKY_WINDOW_RegisterMouse(IN WINDOW _window, IN ADDRESS_SPACE _pdbr, IN fWindowMouseCallback _callback)
{
	if(_window && ENVIRONMENT_OwnsWINDOW(ENVIRONMENT_GetCurrent(), _window) && ENVIRONMENT_OwnsPDBR(ENVIRONMENT_GetCurrent(), _pdbr))
	{
		WINDOW_RegisterMouse(_window, _pdbr, _callback);
	}
}

/**
* @brief Get an arrow pointer handle.
* @return the handle for an arrow pointer.
*/
PUBLIC POINTER	XKY_POINTER_GetArrow()
{
	return POINTER_GetArrow();
}

/**
* @brief Get a clock pointer handle.
* @return the handle for a clock pointer.
*/
PUBLIC POINTER	XKY_POINTER_GetClock()
{
	return POINTER_GetClock();
}

/**
* @brief Sets a pointer for a window.
* @param _window [in] The window.
* @param _pointer [in] The pointer.
*/
PUBLIC void XKY_WINDOW_SetPointer(IN WINDOW _window, IN POINTER _pointer)
{
	if(_window && ENVIRONMENT_OwnsWINDOW(ENVIRONMENT_GetCurrent(), _window))
	{
		WINDOW_SetPointer(_window, _pointer);
	}
}

/**
* @brief Sets the pointer color for a window.
* @param _window [in] The window.
* @param _color [in] The color.
*/
PUBLIC void XKY_WINDOW_SetPointerColor(IN WINDOW _window, IN ARGB _color)
{
	if(_window && ENVIRONMENT_OwnsWINDOW(ENVIRONMENT_GetCurrent(), _window))
	{
		WINDOW_SetPointerColor(_window, _color);
	}
}

//PCI
/**
* @brief Allocates a PCI device.
* @param _device [in] The device we want to allocate.
* @return The PCI address if successful, 0 otherwise.
*/
PUBLIC PCI XKY_PCI_Alloc(IN PCI _device)
{
	//Check it's not used
	for(ENVIRONMENT* e = ENVIRONMENT_First(); e; e = ENVIRONMENT_Next(e))
	{
		if(ENVIRONMENT_OwnsPCI(e, _device))
			return 0;
	}

	//Ok
	if(!ENVIRONMENT_AllocPCI(ENVIRONMENT_GetCurrent(), _device))
		return 0;

	return _device;
}

/**
* @brief Frees a PCI device.
* @param _device [in] The device we want to release.
*/
PUBLIC void XKY_PCI_Free(IN PCI _device)
{
	ENVIRONMENT_FreePCI(ENVIRONMENT_GetCurrent(), _device);
}

/**
* @brief Queries the number of PCI devices.
* @return The number of PCI devices.
*/
PUBLIC dword XKY_PCI_GetNumberOfDevices()
{
	return PCI_GetNumberOfDevices();
}

/**
* @brief Obtains a PCI address of the PCI list given its index.
* @param _index [in] The index of the device.
* @return The PCI device.
*/
PUBLIC PCI	XKY_PCI_GetDevice(IN dword _index)
{
	if(_index < PCI_GetNumberOfDevices())
	{
		return PCI_GetDevice(_index);
	}
	return 0;
}

/**
* @brief Obtains the bus of a PCI address.
* @param _device [in] The PCI device.
* @return The bus of the PCI device.
*/
PUBLIC byte XKY_PCI_GetBusFromDirection(IN PCI _device)
{
	return PCI_GetBusFromDirection(_device);
}

/**
* @brief Obtains the device of a PCI address.
* @param _device [in] The PCI device.
* @return The device of the PCI device.
*/
PUBLIC byte XKY_PCI_GetDeviceFromDirection(IN PCI _device)
{
	return PCI_GetDeviceFromDirection(_device);
}

/**
* @brief Obtains the function of a PCI address.
* @param _device [in] The PCI device.
* @return The function of the PCI device.
*/
PUBLIC byte XKY_PCI_GetFunctionFromDirection(IN PCI _device)
{
	return PCI_GetFunctionFromDirection(_device);
}

/**
* @brief Reads a dword from the address configuration space of a PCI device.
* @param _device [in] The PCI device.
* @param _offset [in] The offset in the address configuration space.
* @return The data at the given offset.
*/
PUBLIC dword XKY_PCI_ReadDword(IN PCI _device, IN dword _offset)
{
	//Check environment has rights
	if(ENVIRONMENT_OwnsPCI(ENVIRONMENT_GetCurrent(), _device))
		return PCI_ReadDword(_device, _offset);

	return 0;
}

/**
* @brief Writes a dword in the address configuration space of a PCI device.
* @param _device [in] The PCI device.
* @param _offset [in] The offset in the address configuration space.
* @param _data [in] The data to write.
*/
PUBLIC void XKY_PCI_WriteDword(IN PCI _device, IN dword _offset, IN dword _data)
{
	//Check environment has rights
	if(ENVIRONMENT_OwnsPCI(ENVIRONMENT_GetCurrent(), _device))
	{
		//Do write
		PCI_WriteDword(_device, _offset, _data);
	}
}

//CPU
/**
* @brief Fills an execution from the given data.
* @param _execution [in] The execution to fill.
* @param _pdbr [in] The address space of the execution.
* @param _code [in] The code of the execution.
* @param _stack [in] The stack of the execution.
* @param _user_mode [in] Indicates if the execution is for user or kernel mode.
*/
PUBLIC void XKY_CPU_FillExecution(IN EXECUTION* _execution, IN ADDRESS_SPACE _pdbr, IN VIRTUAL _code, IN VIRTUAL _stack, IN bool _user_mode)
{
	CPU_FillExecution(_execution, _pdbr, _code, _stack, _user_mode?UserMode:KernelMode);
}

/**
* @brief Allocates a xid for a given execution.
* @param _desired [in] The xid we want, or XID_ANY if does not matter.
* @param _existing [in] The existing xid to copy the execution from.
* @return The allocated xid.
*/
PUBLIC XID XKY_CPU_Alloc(IN XID _desired, IN XID _existing)
{
	//Alloc more CPU
	XID	xid = PROCESSOR_AssignNewExecution(_desired, _existing);
	if(xid)
	{
		if(ENVIRONMENT_AllocCPU(ENVIRONMENT_GetCurrent(), xid))
            return xid;
		
		PROCESSOR_DeleteExecution(xid);
	}
	return 0;
}

/**
* @brief Allocates a xid and a new execution.
* @param _desired [in] The xid we want, or XID_ANY if does not matter.
* @param _pdbr [in] The address space of the new associated-to-xid execution.
* @param _code [in] The code of the new associated-to-xid execution.
* @param _stack [in] The stack of the new associated-to-xid execution.
* @return The allocated xid.
*/
PUBLIC XID XKY_CPU_AllocCode(IN XID _desired, IN ADDRESS_SPACE _pdbr, IN VIRTUAL _code, IN VIRTUAL _stack)
{
	//Alloc more CPU
	EXECUTION* execution = CPU_AllocExecution();
	CPU_FillExecution(execution, _pdbr, _code, _stack, UserMode);
	XID	xid = PROCESSOR_CreateNewExecution(_desired, execution, ENVIRONMENT_GetCurrent());
	if(xid)
	{
		if(ENVIRONMENT_AllocCPU(ENVIRONMENT_GetCurrent(), xid))
            return xid;
		PROCESSOR_DeleteExecution(xid);
	}
	return 0;
}

/**
* @brief Frees a xid.
* @param _xid [in] The xid.
*/
PUBLIC void XKY_CPU_Free(IN XID _xid)
{
	//Release CPU
	if(ENVIRONMENT_FreeCPU(ENVIRONMENT_GetCurrent(), _xid))
	{
		PROCESSOR_DeleteExecution(_xid);
		if(PROCESSOR_GetCurrentXID() == _xid)
		{
			//Wait until a new timer interrupt changes execution
			__asm sti
			for(;;);
		}
	}
}

/**
* @brief Registers a CPU callback for the caller environment.
* @param _pdbr [in] The address space of the callback handler.
* @param _callback [in] The CPU callback.
*/
PUBLIC void XKY_CPU_RegisterCallback(IN ADDRESS_SPACE _pdbr, IN fCpuTimerCallback _callback)
{
	if(ENVIRONMENT_OwnsPDBR(ENVIRONMENT_GetCurrent(), _pdbr))
		ENVIRONMENT_SetTimerCallback(ENVIRONMENT_GetCurrent(), _pdbr, _callback);
}

/**
* @brief Consults the current xid.
* @return The current xid.
*/
PUBLIC XID XKY_CPU_GetCurrent()
{
	return PROCESSOR_GetCurrentXID();
}

//RTC
/**
* @brief Consults the seconds of the system.
* @return The seconds of the system.
*/
PUBLIC byte XKY_RTC_Seconds()
{
	return RTC_Seconds();
}

/**
* @brief Consults the minutes of the system.
* @return The minutes of the system.
*/
PUBLIC byte XKY_RTC_Minutes()
{
	return RTC_Minutes();
}

/**
* @brief Consults the hour of the system.
* @return The hour of the system.
*/
PUBLIC byte XKY_RTC_Hour()
{
	return RTC_Hour();
}

/**
* @brief Consults the day of the week of the system.
* @return The day of the week of the system.
*/
PUBLIC DaysOfWeek XKY_RTC_DayOfWeek()
{
	return RTC_DayOfWeek();
}

/**
* @brief Consults the day of the month of the system.
* @return The day of the month of the system.
*/
PUBLIC byte XKY_RTC_DayOfMonth()
{
	return RTC_DayOfMonth();
}

/**
* @brief Consults the month of the system.
* @return The month of the system.
*/
PUBLIC byte XKY_RTC_Month()
{
	return RTC_Month();
}

/**
* @brief Consults the year of the system.
* @return The year of the system.
*/
PUBLIC dword XKY_RTC_Year()
{
	return RTC_Year();
}

//Timer
/**
* @brief Consults the ticks of the system.
* @return The ticks of the system.
*/
PUBLIC dword XKY_TMR_GetTicks()
{
	return TMR_Ticks();
}

//LDR
/**
* @brief Obtains the address of an exported function.
* @param _module [in] The address of the module where the exported function is located.
* @param _function_name [in] The name of the function.
* @return The virtual address of the function, 0 if error.
*/
PUBLIC VIRTUAL XKY_LDR_GetProcedureAddress(IN VIRTUAL _module, IN string* _function_name)
{
	return LDR_GetProcedureAddress((IMG_MODULE_HEADER*)_module, _function_name);
}

/**
* @brief Loads a user mode module in an address space.
* @param _module [in] The name of the module.
* @param _pdbr [in] The address space where to map the module.
* @param _base [in] The virtual base address of the module.
* @return True if successful, false otherwise.
*/
PUBLIC bool XKY_LDR_LoadUserModule(IN string* _module, IN ADDRESS_SPACE _pdbr, IN VIRTUAL _base)
{
	if(FILE_Exists(_module))
	{
		//Get parameters
		STRING_Copy(dynamic_module_name, _module);

		//Change to kernel memory space to be able to map in environments memory address
		ADDRESS_SPACE current = ADDRESS_SPACE_GetCurrent();
		ADDRESS_SPACE_ResetToKernelSpace();

		//Map Module
		PHYSICAL module = LDR_LoadImage(dynamic_module_name, UserMode);
		if(module)
		{
			dword file_size = FILE_Size(dynamic_module_name);
			dword pages_to_map = RTL_BytesToPages(file_size);

			if(ADDRESS_SPACE_Map(_pdbr, module, _base, pages_to_map, UserMode, ReadWrite, true))
			{
				//Reubicate
				LDR_ReubicateImage((IMG_MODULE_HEADER*)module, _base);

				//Resolve imports with user api module
				PTE* pte_api = VIRTUAL_PTE_Address(_pdbr, API_START_DIRECTION);

				if(LDR_ResolveImports((IMG_MODULE_HEADER*)module, (IMG_MODULE_HEADER*) (pte_api->address<<12)))
				{
					//Restore memory space
					ADDRESS_SPACE_SwitchTo(current);
					return true;
				}
				else
				{
					MEM_ReleasePages(module, pages_to_map);
				}
			}
			else
			{
				MEM_ReleasePages(module, pages_to_map);
			}
		}

		//Restore memory space
		ADDRESS_SPACE_SwitchTo(current);
	}
	return false;
}

/**
* @brief Loads a kernel mode module.
* @param _module [in] The name of the module.
* @return True if successful, false otherwise.
*/
PUBLIC bool XKY_LDR_LoadKernelModule(IN string* _module)
{
	if(FILE_Exists(_module))
	{
		//Get parameters
		STRING_Copy(dynamic_module_name, _module);

		//Map Module
		PHYSICAL module = LDR_LoadImage(dynamic_module_name, KernelMode);
		if(module)
		{
			dword file_size = FILE_Size(dynamic_module_name);
			dword pages_to_map = RTL_BytesToPages(file_size);

			//Reubicate
			LDR_ReubicateImage((IMG_MODULE_HEADER*)module, (dword)module);

			//Resolve imports with user api module
#define KERNEL_LOAD_ADDRESS 0x00030000
			if(LDR_ResolveImports((IMG_MODULE_HEADER*)module, (IMG_MODULE_HEADER*)KERNEL_LOAD_ADDRESS))
			{
				//Now find EntryPoint and call it
				IMG_MODULE_HEADER* header = (IMG_MODULE_HEADER*)module;
				if(header->file_header.code_ep)
				{
					return ((bool (*)(void))header->file_header.code_ep)();
				}
				else
				{
					return true;
				}
			}
			else
			{
				MEM_ReleasePages(module, pages_to_map);
			}
		}
	}
	return false;
}

/**
* @brief Loads a file in an address space.
* @param _file [in] The name of the file.
* @param _pdbr [in] The address space where to map the file.
* @param _base [in] The virtual base address of the file.
* @return True if successful, false otherwise.
*/
PUBLIC bool XKY_LDR_LoadFile(IN string* _file, IN ADDRESS_SPACE _pdbr, IN VIRTUAL _base)
{
	if(FILE_Exists(_file))
	{
		//Get parameters
		STRING_Copy(dynamic_module_name, _file);

		//Change to kernel memory space to be able to map in environments memory address
		ADDRESS_SPACE current = ADDRESS_SPACE_GetCurrent();
		ADDRESS_SPACE_ResetToKernelSpace();

		//Map Module
		PHYSICAL module = LDR_LoadFile(dynamic_module_name, UserMode);
		if(module)
		{
			dword file_size = FILE_Size(dynamic_module_name);
			dword pages_to_map = RTL_BytesToPages(file_size);

			if(ADDRESS_SPACE_Map(_pdbr, module, _base, pages_to_map, UserMode, ReadWrite, true))
			{
				ADDRESS_SPACE_SwitchTo(current);
				return true;
			}
			else
			{
				MEM_ReleasePages(module, pages_to_map);
			}
		}

		//Restore memory space
		ADDRESS_SPACE_SwitchTo(current);
	}
	return false;
}

/**
* @brief Queries the size of a file.
* @param _file [in] The name of the file.
* @return The file size.
*/
PUBLIC dword XKY_LDR_FileSize(IN string* _file)
{
	return FILE_Size(_file);
}

//OS
/**
* @brief Starts an application context.
* @param _module [in] The name of the module.
* @return True if successful, false otherwise.
*/
PUBLIC bool XKY_OS_Start(IN string* _module)
{
	if(FILE_Exists(_module))
	{
		//Get parameters
		STRING_Copy(kernel_module_name, _module);

		//Change to kernel memory space to be able to map in environments memory address
		ADDRESS_SPACE current = ADDRESS_SPACE_GetCurrent();
		ADDRESS_SPACE_ResetToKernelSpace();

		//Create new process
		ENVIRONMENT* environment = ENVIRONMENT_Create(kernel_module_name);

		//Restore memory space
		ADDRESS_SPACE_SwitchTo(current);

		return environment != 0;
	}
	return 0;
}

/**
* @brief Finalizes the caller environment.
*/
PUBLIC void XKY_OS_Finish()
{
	//Free environment
	ENVIRONMENT_Release(ENVIRONMENT_GetCurrent());

	//Wait until a new timer interrupt changes execution
	__asm sti
	for(;;);
}

//EXCEPTION
/**
* @brief Registers an exeception handler.
* @param _exception [in] The exception vector.
* @param _pdbr [in] The address space of the handler.
* @param _handler [in] The exception handler.
* @return True if successful, false otherwise.
*/
PUBLIC bool XKY_EXCEPTION_SetHandler(IN byte _exception, IN ADDRESS_SPACE _pdbr, IN fInterruptHandler _handler)
{
	if(ENVIRONMENT_OwnsPDBR(ENVIRONMENT_GetCurrent(), _pdbr))
		return ENVIRONMENT_SetExceptionHandler(ENVIRONMENT_GetCurrent(), _exception, _pdbr, _handler);
	return false;
}

/**
* @brief Unregisters an exception handler.
* @param _exception [in] The exception vector.
*/
PUBLIC void XKY_EXCEPTION_UnsetHandler(IN byte _exception)
{
	ENVIRONMENT_UnsetExceptionHandler(ENVIRONMENT_GetCurrent(), _exception);
}

/**
* @brief Registers an interrupt handler.
* @param _interrupt [in] The interrupt vector.
* @param _handler [in] The interrupt handler.
* @return True if successful, false otherwise.
*/
PUBLIC bool XKY_INTERRUPT_SetHandler(IN byte _interrupt, IN fInterruptHandler _handler)
{
	return INT_SetHandler(SoftwareInterrupt, _interrupt, _handler);
}

/**
* @brief Unregisters an interrupt handler.
* @param _interrupt [in] The interrupt vector.
* @param _handler [in] The interrupt handler.
* @return True if successful, false otherwise.
*/
PUBLIC void XKY_INTERRUPT_UnsetHandler(IN byte _interrupt, IN fInterruptHandler _handler)
{
	INT_UnsetHandler(SoftwareInterrupt, _interrupt, _handler);
}

//DEBUG
/**
* @brief Sends a message to the debug output.
* @param _message [in] The message to show.
* @param _color [in] The color of the text.
*/
PUBLIC void XKY_DEBUG_Message(IN string* _message, IN dword _color)
{
	DEBUG_Message(_message, _color);
}

/**
* @brief Sends a message and data to the debug output.
* @param _message [in] The message to show.
* @param _data [in] The data to show.
* @param _color [in] The color of the text.
*/
PUBLIC void XKY_DEBUG_Data(IN string* _message, IN dword _data, IN dword _color)
{
	DEBUG_Data(_message, _data, _color);
}
