/******************************************************************************/
/**
* @file		Environment.cpp
* @brief	XkyOS Environments definitions
* Implementation of the running elements in XkyOS, which have account information
* 
* @date		20/03/2008
* @author	Pablo Bravo
*/
/******************************************************************************/
#include "Environment.h"
#include "RTL.h"

#include "Debug.h"

//==================================DATA======================================//
#pragma data_seg(".data")
//============================================================================//
#define MAX_EXCEPTIONS			32	/**< Number of exceptions */
//#define ENVIRONMENT_NAME_SIZE	63	/**< Max size of an environment name */

/**
* @brief The "smart" exception handler.
* Can invoke an exception handler in an address space independent way.
*/
struct EXCEPTION_HANDLER
{
	ADDRESS_SPACE		pdbr;
	fInterruptHandler	handler;
};

/**
* @brief The "smart" timer callback.
* Can invoke a timer callback in an address space independent way.
*/
struct TIMER_CALLBACK
{
	ADDRESS_SPACE		pdbr;
	fCpuTimerCallback	callback;
};

/**
* @brief The environment structure.
*/
struct ENVIRONMENT
{
	/**
	* @brief This makes the environment able to be in a list.
	*/
	LIST_ENTRY listable;
	/**
	* @brief The memory address space of the environment.
	*/
	LIST_ENTRY pdbrs;
	/**
	* @brief The list of disk sectors for the environment.
	*/
	LIST_ENTRY disks;
	/**
	* @brief The list of cpu slices for the environment.
	*/
	LIST_ENTRY cpus;
	/**
	* @brief The list of windows for the environment.
	*/
	LIST_ENTRY windows;
	/**
	* @brief The list of pci devices for the environment.
	*/
	LIST_ENTRY pcis;
	/**
	* @brief The list of exception handlers associated with the process.
	*/
	EXCEPTION_HANDLER exceptions[MAX_EXCEPTIONS];
	/**
	* @brief The timer callback, if any.
	*/
	TIMER_CALLBACK timer;
	/**
	* @brief Name of the process.
	*/
	//DEFINE_BOUNDED_STRING(name, ENVIRONMENT_NAME_SIZE);
};

/**
* @brief The structure where environment resources gets tracked.
*/
struct RESOURCE
{
	/**
	* @brief This makes the resource able to be in a list.
	*/
	LIST_ENTRY listable;
	/**
	* @brief Resource type.
	*/
	union
	{
		ADDRESS_SPACE	pdbr;
		DISK_RANGE		disk_range;
		XID				xid;
		WINDOW			window;
		PCI				pci;
	};
};

/**
* @brief The environments list.
*/
PRIVATE LIST_ENTRY environments;
/**
* @brief The current environment.
*/
PRIVATE ENVIRONMENT* current_environment = 0;
/**
* @brief User mode API module name.
*/
PRIVATE string api_name = STRING("API.x");

//==================================CODE======================================//
#pragma code_seg(".code")
//============================================================================//
/**
* @brief Actualizes current environment.
* @param _frame [in] The interrupt frame.
* @return True to indicate a change in previous cpu state.
*/
PRIVATE bool EnvironmentProcessorCallback(IN XID _xid, IN EXECUTION* _execution, IN ENVIRONMENT* _environment)
{
	//Actualize the current environment
	if(_environment != current_environment)
	{
		current_environment = _environment;
	}
	if(current_environment)
	{
		//Call GUI callbacks
		for(LIST_ITERATOR iter = LIST_First(&current_environment->windows); iter; iter = LIST_Next(&current_environment->windows, iter))
		{
			RESOURCE* node = (RESOURCE*)iter;
			WINDOW_FlushKeyboardCallbacks(node->window);
			WINDOW_FlushMouseCallbacks(node->window);
		}

		//Call timer callback
		if(current_environment->timer.callback && current_environment->timer.pdbr)
		{
			ADDRESS_SPACE current = ADDRESS_SPACE_GetCurrent();
			ADDRESS_SPACE_SwitchTo(current_environment->timer.pdbr);

			bool changed = current_environment->timer.callback(_xid, _execution);

			ADDRESS_SPACE_SwitchTo(current);
			return changed;
		}
	}
	return false;
}

/**
* @brief Initialize environment support.
* @return True for continuing interrupt chain.
*/
PUBLIC bool ENVIRONMENT_Init()
{
	//Initialize environments list.
	LIST_Init(&environments);

	//Set current environment to none
	current_environment = 0;

	return true;
}

/**
* @brief Returns the current executing environment.
* @param _module_name [in] The module that will create an environment.
* @return Newly created environment, or zero if error.
*/
PUBLIC ENVIRONMENT* ENVIRONMENT_Create(IN string* _module_name)
{
	//Allocate memory for the environment
	ENVIRONMENT* environment = (ENVIRONMENT*)HEAP_Alloc(sizeof(ENVIRONMENT));
	if(!environment)
	{
		DEBUG("No HEAP_Alloc for environment")
		return 0;
	}

	//Prepare environment to be enlisted
	LIST_Init((LIST_ENTRY*)environment);
	
	//Get the initial address space
	ADDRESS_SPACE initial_pdbr = ADDRESS_SPACE_Create();
	if(!initial_pdbr)
	{
		DEBUG("No ADDRESS_SPACE_Create for environment")
		//Free the environment
		HEAP_Free((PHYSICAL&)environment);
		return 0;
	}

	//Initialize resource lists
	LIST_Init(&environment->pdbrs);
	LIST_Init(&environment->disks);
	LIST_Init(&environment->cpus);
	LIST_Init(&environment->windows);
	LIST_Init(&environment->pcis);

	//Add initial_pdbr to the list of environment's address spaces
	if(!ENVIRONMENT_AllocPDBR(environment, initial_pdbr))
	{
		goto _Error;
	}

	//Initialize exceptions
	for(dword i = 0; i < MAX_EXCEPTIONS; i++)
	{
		environment->exceptions[i].handler = 0;
		environment->exceptions[i].pdbr = 0;
	}

	//Initialize timer
	for(dword i = 0; i < MAX_EXCEPTIONS; i++)
	{
		environment->timer.callback = 0;
		environment->timer.pdbr = 0;
	}

	//Load api
	PHYSICAL api_module = LDR_LoadImage(&api_name, UserMode);
	if(!api_module)
	{
		DEBUG("No LDR_LoadImage API")
		goto _Error;
	}

	//Map and rebase the api
	dword api_pages_to_map = RTL_BytesToPages(FILE_Size(&api_name));

	if(!ADDRESS_SPACE_Map(initial_pdbr, api_module, API_START_DIRECTION, api_pages_to_map, UserMode, ReadOnly, true))
	{
		MEM_ReleasePages(api_module, api_pages_to_map);
		goto _Error;
	}

	LDR_ReubicateImage((IMG_MODULE_HEADER*)api_module, API_START_DIRECTION);

	//Load module image
	PHYSICAL exec_module = LDR_LoadImage(_module_name, UserMode);

	if(!exec_module)
	{
		DEBUG_DATA("No LDR_LoadImage EXE = ", exec_module, 0x0000FF00)
		//Free all (note that api module is mapped, so this wil release physical pages allocated in Map)
		goto _Error;
	}

	/*
	IMPORTANT:
		Module api must be rebased before resolving imports so the function addresses are the
		final virtual ones.
	*/

	//Resolve imports
	dword exec_pages_to_map = RTL_BytesToPages(FILE_Size(_module_name));

	if(!LDR_ResolveImports((IMG_MODULE_HEADER*)exec_module, (IMG_MODULE_HEADER*)api_module))
	{
		DEBUG("No LDR_ResolveImports")
		//Free all
		MEM_ReleasePages(exec_module, exec_pages_to_map);
		goto _Error;
	}

	//Map and rebase
	if(!ADDRESS_SPACE_Map(initial_pdbr, exec_module, MODULE_START_DIRECTION, exec_pages_to_map, UserMode, ReadWrite, true))
	{
		MEM_ReleasePages(exec_module, exec_pages_to_map);
		goto _Error;
	}

	LDR_ReubicateImage((IMG_MODULE_HEADER*)exec_module, MODULE_START_DIRECTION);

	//Allocate and map stack
	PHYSICAL stack = MEM_AllocPages(NUMBER_OF_STACK_PAGES, UserMode);
	if(!stack) goto _Error;

	if(!ADDRESS_SPACE_Map(initial_pdbr, stack, STACK_START_DIRECTION, NUMBER_OF_STACK_PAGES, UserMode, ReadWrite, true))
	{
		MEM_ReleasePages(stack, NUMBER_OF_STACK_PAGES);
		goto _Error;
	}
	
	//Create execution
	EXECUTION* execution = CPU_AllocExecution();
	if(!execution) goto _Error;

	CPU_FillExecution(execution, initial_pdbr, ((IMG_MODULE_HEADER*)exec_module)->file_header.code_ep, STACK_START_DIRECTION + PAGE_SIZE*NUMBER_OF_STACK_PAGES - 4, UserMode);

	XID xid = PROCESSOR_CreateNewExecution(XID_ANY, execution, environment);
	if(!xid)
	{
		DEBUG("No CPU_CreateNewExecution")
		//Free all
		CPU_FreeExecution(execution);
		goto _Error;
	}

	//Add XID resource and register the callback
	if(!ENVIRONMENT_AllocCPU(environment, xid))
	{
		DEBUG("No ENVIRONMENT_AllocXID")
		//Free all
		PROCESSOR_DeleteExecution(xid);
		goto _Error;
	}

	//Insert in environment list
	LIST_InsertTail(&environments, (LIST_ENTRY*)environment);

	//Done
	return environment;

_Error:
	ADDRESS_SPACE_Release(initial_pdbr);
	HEAP_Free((PHYSICAL&)environment);
	return 0;
}

/**
* @brief Frees a reource environment list.
* @param _head [in] The head of the reosurce list.
*/
PRIVATE void ENVIRONMENT_DeleteList(IN LIST_ENTRY* _head)
{
	while(!LIST_IsEmpty(_head))
	{
		LIST_ITERATOR iter = LIST_First(_head);
		LIST_Remove(iter);
		HEAP_Free((PHYSICAL&)iter); //RESOURCE*
	}
}

/**
* @brief Deletes an environment.
* @param _environment [in] The environment to release.
*/
PUBLIC void ENVIRONMENT_Release(IN ENVIRONMENT* _environment)
{
	ADDRESS_SPACE_ResetToKernelSpace();

	//Free resources
	//Windows
	LIST_ITERATOR iter;
	for(iter = LIST_First(&_environment->windows); iter; iter = LIST_Next(&_environment->windows, iter))
	{
		RESOURCE* node = (RESOURCE*)iter;
		WINDOW_ReleaseWindow(node->window);
	}
	//Xid's
	for(iter = LIST_First(&_environment->cpus); iter; iter = LIST_Next(&_environment->cpus, iter))
	{
		RESOURCE* node = (RESOURCE*)iter;
		PROCESSOR_DeleteExecution(node->xid);
	}
	//PDBR's
	for(iter = LIST_First(&_environment->pdbrs); iter; iter = LIST_Next(&_environment->pdbrs, iter))
	{
		RESOURCE* node = (RESOURCE*)iter;
		ADDRESS_SPACE_Release(node->pdbr);
	}
	//DISK
	/*
	for(iter = LIST_First(&_environment->disks); iter; iter = LIST_Next(&_environment->disks, iter))
	{
		RESOURCE* node = (RESOURCE*)iter;
		DISK_RANGE_Release(node->disk_range);
		//Actualize persistent disk owning list if applicable
	}
	*/

	//Delete the lists
	ENVIRONMENT_DeleteList(&_environment->pdbrs);
	ENVIRONMENT_DeleteList(&_environment->disks);
	ENVIRONMENT_DeleteList(&_environment->cpus);
	ENVIRONMENT_DeleteList(&_environment->windows);
	ENVIRONMENT_DeleteList(&_environment->pcis);

	//Remove environment from list
	LIST_Remove((LIST_ENTRY*)_environment);

	//Free the environment
	HEAP_Free((PHYSICAL&)_environment);
}

/**
* @brief Assigns an address space to an environment.
* @param _environment [in] The environment.
* @param _address_space [in] The address space.
* @return True if assignment can be done, false otherwise.
*/
PUBLIC bool ENVIRONMENT_AllocPDBR(IN ENVIRONMENT* _environment, IN ADDRESS_SPACE _address_space)
{
	if(ENVIRONMENT_OwnsPDBR(_environment, _address_space))
		return false;

	RESOURCE* node = (RESOURCE*)HEAP_Alloc(sizeof(RESOURCE));
	if(!node) return false;

	node->pdbr = _address_space;
	LIST_Init((LIST_ENTRY*)node);
	LIST_InsertTail(&_environment->pdbrs, (LIST_ENTRY*)node);

	return true;
}

/**
* @brief Indicates if a given address space is owned by an environment.
* @param _environment [in] The environment.
* @param _address_space [in] The address space.
* @return True if the address space is owned.
*/
PUBLIC bool ENVIRONMENT_OwnsPDBR(IN ENVIRONMENT* _environment, IN ADDRESS_SPACE _address_space)
{
	for(LIST_ITERATOR iter = LIST_First(&_environment->pdbrs); iter; iter = LIST_Next(&_environment->pdbrs, iter))
	{
		RESOURCE* node = (RESOURCE*)iter;
		if(node->pdbr == _address_space)
			return true;
	}
	return false;
}

/**
* @brief Liberates an address space.
* @param _environment [in] The environment.
* @param _address_space [in] The address space to release.
* @return True if the address space was owned.
*/
PUBLIC bool ENVIRONMENT_FreePDBR(IN ENVIRONMENT* _environment, IN ADDRESS_SPACE _address_space)
{
	for(LIST_ITERATOR iter = LIST_First(&_environment->pdbrs); iter; iter = LIST_Next(&_environment->pdbrs, iter))
	{
		RESOURCE* node = (RESOURCE*)iter;
		if(node->pdbr == _address_space)
		{
			LIST_Remove(iter);
			HEAP_Free((PHYSICAL&)node);
			return true;
		}
	}
	return false;
}

/**
* @brief Assigns a disk range to an environment.
* @param _environment [in] The environment.
* @param _disk_range [in] The disk range of sectors.
* @return True if assignment can be done, false otherwise.
*/
PUBLIC bool ENVIRONMENT_AllocDISK(IN ENVIRONMENT* _environment, IN DISK_RANGE _disk_range)
{
	if(ENVIRONMENT_OwnsDISK(_environment, _disk_range))
		return false;

	RESOURCE* node = (RESOURCE*)HEAP_Alloc(sizeof(RESOURCE));
	if(!node) return false;

	node->disk_range = _disk_range;
	LIST_Init((LIST_ENTRY*)node);
	LIST_InsertTail(&_environment->disks, (LIST_ENTRY*)node);
	return true;
}

/**
* @brief Indicates if a given range of disk addresses is owned by an environment.
* @param _environment [in] The environment.
* @param _disk_range [in] The range of sector addresses to test.
* @return True if the any of the sectors is owned.
*/
PUBLIC bool ENVIRONMENT_OwnsDISK(IN ENVIRONMENT* _environment, IN DISK_RANGE _disk_range)
{
	for(LIST_ITERATOR iter = LIST_First(&_environment->disks); iter; iter = LIST_Next(&_environment->disks, iter))
	{
		RESOURCE* node = (RESOURCE*)iter;
		if(DISK_RANGE_ClashWith(node->disk_range, _disk_range))
			return true;
	}
	return false;
}

/**
* @brief Liberates a range of sectors.
* @param _environment [in] The environment.
* @param _start_lba [in] The starting lba of the disk range to release.
* @return True if the lba was owned.
*/
PUBLIC bool ENVIRONMENT_FreeDISK(IN ENVIRONMENT* _environment, IN LBA _start_lba)
{
	for(LIST_ITERATOR iter = LIST_First(&_environment->disks); iter; iter = LIST_Next(&_environment->disks, iter))
	{
		RESOURCE* node = (RESOURCE*)iter;
		if(DISK_RANGE_Contains(node->disk_range, _start_lba))
		{
			LIST_Remove(iter);
			HEAP_Free((PHYSICAL&)node);
			return true;
		}
	}
	return false;
}

/**
* @brief Assigns a xid to an environment.
* @param _environment [in] The environment.
* @param _xid [in] The xid.
* @return True if assignment can be done, false otherwise.
*/
PUBLIC bool ENVIRONMENT_AllocCPU(IN ENVIRONMENT* _environment, IN XID _xid)
{
	if(ENVIRONMENT_OwnsCPU(_environment, _xid))
		return false;

	RESOURCE* node = (RESOURCE*)HEAP_Alloc(sizeof(RESOURCE));
	if(!node) return false;

	node->xid = _xid;
	LIST_Init((LIST_ENTRY*)node);
	LIST_InsertTail(&_environment->cpus, (LIST_ENTRY*)node);

	PROCESSOR_RegisterCallback(_xid, EnvironmentProcessorCallback);

	return true;
}

/**
* @brief Indicates if a given xid is owned by an environment.
* @param _environment [in] The environment.
* @param _xid [in] The xid to test.
* @return True if the xid is owned.
*/
PUBLIC bool ENVIRONMENT_OwnsCPU(IN ENVIRONMENT* _environment, IN XID _xid)
{
	for(LIST_ITERATOR iter = LIST_First(&_environment->cpus); iter; iter = LIST_Next(&_environment->cpus, iter))
	{
		RESOURCE* node = (RESOURCE*)iter;
		if(node->xid == _xid)
			return true;
	}
	return false;
}

/**
* @brief Liberates a xid.
* @param _environment [in] The environment.
* @param _xid [in] The xid to release.
* @return True if the xid was owned.
*/
PUBLIC bool ENVIRONMENT_FreeCPU(IN ENVIRONMENT* _environment, IN XID _xid)
{
	for(LIST_ITERATOR iter = LIST_First(&_environment->cpus); iter; iter = LIST_Next(&_environment->cpus, iter))
	{
		RESOURCE* node = (RESOURCE*)iter;
		if(node->xid == _xid)
		{
			LIST_Remove(iter);
			HEAP_Free((PHYSICAL&)node);
			return true;
		}
	}
	return false;
}

/**
* @brief Assigns a window to an environment.
* @param _environment [in] The environment.
* @param _window [in] The window.
* @return True if assignment can be done, false otherwise.
*/
PUBLIC bool ENVIRONMENT_AllocWINDOW(IN ENVIRONMENT* _environment, IN WINDOW _window)
{
	if(ENVIRONMENT_OwnsWINDOW(_environment, _window))
		return false;

	RESOURCE* node = (RESOURCE*)HEAP_Alloc(sizeof(RESOURCE));
	if(!node) return false;

	node->window = _window;
	LIST_Init((LIST_ENTRY*)node);
	LIST_InsertTail(&_environment->windows, (LIST_ENTRY*)node);
	return true;
}

/**
* @brief Indicates if a given window is owned by an environment.
* @param _environment [in] The environment.
* @param _window [in] The window to test.
* @return True if the window is owned.
*/
PUBLIC bool ENVIRONMENT_OwnsWINDOW(IN ENVIRONMENT* _environment, IN WINDOW _window)
{
	for(LIST_ITERATOR iter = LIST_First(&_environment->windows); iter; iter = LIST_Next(&_environment->windows, iter))
	{
		RESOURCE* node = (RESOURCE*)iter;
		if(node->window == _window)
			return true;
	}
	return false;
}

/**
* @brief Liberates a window.
* @param _environment [in] The environment.
* @param _window [in] The window to release.
* @return True if the window was owned.
*/
PUBLIC bool ENVIRONMENT_FreeWINDOW(IN ENVIRONMENT* _environment, IN WINDOW _window)
{
	for(LIST_ITERATOR iter = LIST_First(&_environment->windows); iter; iter = LIST_Next(&_environment->windows, iter))
	{
		RESOURCE* node = (RESOURCE*)iter;
		if(node->window == _window)
		{
			LIST_Remove(iter);
			HEAP_Free((PHYSICAL&)node);
			return true;
		}
	}
	return false;
}

/**
* @brief Assigns a pci device to an environment.
* @param _environment [in] The environment.
* @param _pci [in] The pci address device.
* @return True if assignment can be done, false otherwise.
*/
PUBLIC bool ENVIRONMENT_AllocPCI(IN ENVIRONMENT* _environment, IN PCI _pci)
{
	if(ENVIRONMENT_OwnsPCI(_environment, _pci))
		return false;

	RESOURCE* node = (RESOURCE*)HEAP_Alloc(sizeof(RESOURCE));
	if(!node) return false;

	node->pci = _pci;
	LIST_Init((LIST_ENTRY*)node);
	LIST_InsertTail(&_environment->pcis, (LIST_ENTRY*)node);
	return true;
}

/**
* @brief Indicates if a given pci device is owned by an environment.
* @param _environment [in] The environment.
* @param _pci [in] The pci address to test.
* @return True if the pci device is owned.
*/
PUBLIC bool ENVIRONMENT_OwnsPCI(IN ENVIRONMENT* _environment, IN PCI _pci)
{
	for(LIST_ITERATOR iter = LIST_First(&_environment->pcis); iter; iter = LIST_Next(&_environment->pcis, iter))
	{
		RESOURCE* node = (RESOURCE*)iter;
		if(node->pci == _pci)
			return true;
	}
	return false;
}

/**
* @brief Liberates a pci device.
* @param _environment [in] The environment.
* @param _pci [in] The pci device address to release.
* @return True if the pci device was owned.
*/
PUBLIC bool ENVIRONMENT_FreePCI(IN ENVIRONMENT* _environment, IN PCI _pci)
{
	for(LIST_ITERATOR iter = LIST_First(&_environment->pcis); iter; iter = LIST_Next(&_environment->pcis, iter))
	{
		RESOURCE* node = (RESOURCE*)iter;
		if(node->pci == _pci)
		{
			LIST_Remove(iter);
			HEAP_Free((PHYSICAL&)node);
			return true;
		}
	}
	return false;
}

/**
* @brief Returns the current executing environment.
* @return Current environment.
*/
PUBLIC ENVIRONMENT* ENVIRONMENT_GetCurrent()
{
	return current_environment;
}

/**
* @brief Walks through the environments list. Gets first element.
* @return First environment, if any, zero otherwise.
*/
PUBLIC ENVIRONMENT* ENVIRONMENT_First()
{
	return (ENVIRONMENT*)LIST_First(&environments);
}

/**
* @brief Walks through the environments list. Gets next element to a given one.
* @param _last_environment [in] The environment obtained in a previous call.
* @return An environment, or zero if parameter environment was last in list.
*/
PUBLIC ENVIRONMENT* ENVIRONMENT_Next(IN ENVIRONMENT* _last_environment)
{
	return (ENVIRONMENT*)LIST_Next(&environments, (LIST_ITERATOR)_last_environment);
}

/**
* @brief Set a timer callback for the environment.
* @param _environment [in] The environment.
* @param _pdbr [in] The address space.
* @param _callback [in] The callback.
*/
PUBLIC void ENVIRONMENT_SetTimerCallback(IN ENVIRONMENT* _environment, IN ADDRESS_SPACE _pdbr, IN fCpuTimerCallback _callback)
{
	_environment->timer.pdbr = _pdbr;
	_environment->timer.callback = _callback;
}

/**
* @brief Unsets the timer callback for the environment.
* @param _environment [in] The environment.
*/
PUBLIC void ENVIRONMENT_UnsetTimerCallback(IN ENVIRONMENT* _environment)
{
	_environment->timer.pdbr = 0;
	_environment->timer.callback = 0;
}

/**
* @brief Set a given exception handler for an environment.
* @param _environment [in] The environment.
* @param _exception [in] The exception to handle.
* @param _handler [in] The handler wich will give service.
* @return True if exception was set, false otherwise.
*/
PUBLIC bool ENVIRONMENT_SetExceptionHandler(IN ENVIRONMENT* _environment, IN byte _exception, IN ADDRESS_SPACE _pdbr, IN fInterruptHandler _handler)
{
	if(_exception < 32)
	{
		_environment->exceptions[_exception].handler = _handler;
		_environment->exceptions[_exception].pdbr = _pdbr;
		return true;
	}
	return false;
}

/**
* @brief Eliminates a given exception handler for an environment.
* @param _environment [in] The environment.
* @param _exception [in] The exception to handle.
*/
PUBLIC void ENVIRONMENT_UnsetExceptionHandler(IN ENVIRONMENT* _environment, IN byte _exception)
{
	if(_exception < 32)
	{
		_environment->exceptions[_exception].handler = 0;
		_environment->exceptions[_exception].pdbr = 0;
	}
}

/**
* @brief Executes the _execption indicated exception handler, if any.
* @param _environment [in] The environment.
* @param _exception [in] The exception to handle.
*/
PUBLIC bool ENVIRONMENT_InvokeHandler(IN ENVIRONMENT* _environment, IN byte _exception, IN INTERRUPT_FRAME* _frame)
{
	if((_exception) < 32 && _environment->exceptions[_exception].handler && _environment->exceptions[_exception].pdbr)
	{
		ADDRESS_SPACE current = ADDRESS_SPACE_GetCurrent();
		ADDRESS_SPACE_SwitchTo(_environment->exceptions[_exception].pdbr);

		bool exception_resolved = _environment->exceptions[_exception].handler(_frame);

		ADDRESS_SPACE_SwitchTo(current);
		return exception_resolved;
	}
	return false;
}
