/******************************************************************************/
/**
* @file		Kernel.cpp
* @brief	XkyOS Kernel Module
* Entry point, Initialization and setup of XkyOS
* 
* @date		20/03/2008
* @author	Pablo Bravo
*/
/******************************************************************************/

#include "Types.h"
#include "System.h"
#include "Executable.h"
#include "Loader.h"
#include "Hardware.h"
#include "RTL.h"
#include "Exported.h"
#include "Environment.h"
#include "Functions.h"

#include "Debug.h"

//=================================IMPORTS====================================//
#pragma data_seg(".imports")
//============================================================================//

//==================================DATA======================================//
#pragma data_seg(".data")
//============================================================================//
string CLI = STRING("CLI.x");

//==================================CODE======================================//
#pragma code_seg(".code")
//============================================================================//
/**
* @brief Kernel error.
* @param _frame [in] The interrupt frame.
* @return Never returns.
*/
PRIVATE bool INTERRUPT KERNEL_Bug(IN INTERRUPT_FRAME* _frame)
{
	//Full screen
	//SVGA_ClearScreen(SRGB(255,0,0));
	//DEBUG_Uninit();

	//Debug Console
	DEBUG_Clear(SRGB(255, 0, 0));

	//Show Data
	dword cr3 = CPU_ReadCR3();
	ADDRESS_SPACE_ResetToKernelSpace();

	//Exception information
	DEBUG("SYSTEM ERROR");
	DEBUG_DATA("  Exception = ", _frame->vector, SRGB(0,0,0));
	DEBUG_DATA("  Error     = ", _frame->error, SRGB(0,0,0));

	//Segments
	DEBUG("SEGMENT REGISTERS");
	DEBUG_DATA("  CS = ", _frame->cs, SRGB(0,0,0));
	DEBUG_DATA("  SS = ", _frame->ss, SRGB(0,0,0));
	DEBUG_DATA("  DS = ", _frame->ds, SRGB(0,0,0));
	DEBUG_DATA("  ES = ", _frame->es, SRGB(0,0,0));
	DEBUG_DATA("  FS = ", _frame->fs, SRGB(0,0,0));
	DEBUG_DATA("  GS = ", _frame->gs, SRGB(0,0,0));

	//Control
	DEBUG("CONTROL REGISTERS");
	DEBUG_DATA("  CR2 = ", CPU_ReadCR2(), SRGB(0,0,0));
	DEBUG_DATA("  CR3 = ", cr3, SRGB(0,0,0));

	//Common
	DEBUG("COMMON REGISTERS");
	DEBUG_DATA("  EFLAGS = ", _frame->eflags, SRGB(0,0,0));
	DEBUG_DATA("  EIP    = ", _frame->eip, SRGB(0,0,0));
	DEBUG_DATA("  EAX    = ", _frame->eax, SRGB(0,0,0));
	DEBUG_DATA("  EBX    = ", _frame->ebx, SRGB(0,0,0));
	DEBUG_DATA("  ECX    = ", _frame->ecx, SRGB(0,0,0));
	DEBUG_DATA("  EDX    = ", _frame->edx, SRGB(0,0,0));
	DEBUG_DATA("  ESI    = ", _frame->esi, SRGB(0,0,0));
	DEBUG_DATA("  EDI    = ", _frame->edi, SRGB(0,0,0));
	DEBUG_DATA("  EBP    = ", _frame->ebp, SRGB(0,0,0));

	if(INT_InterruptFromUserMode(_frame))
	{
		DEBUG_DATA("  ESP    = ", _frame->esp, SRGB(0,0,0));
	}
	else
	{
		DEBUG_DATA("  ESP    = ", _frame->kernel_esp, SRGB(0,0,0));
	}

	//Halt system
	__asm cli
	__asm hlt

	//Never reached
	return true;
}

/**
* @brief User mode interrupt handler for services requests.
* @param _frame [in] The interrupt frame.
* @return True if the petition couldn't be served for further processing, false if served.
*/
PRIVATE bool INTERRUPT KERNEL_Services(IN INTERRUPT_FRAME* _frame)
{
	//EAX has service index.
	dword service = _frame->eax;
	_frame->eax = 0;

	//Stack
	dword* stack = 0;

	//Check stack
	if(INT_InterruptFromUserMode(_frame))
	{
		//Get user mode stack
		stack = (dword*)_frame->esp;
	}
	else
	{
		//Get kernel mode stack
		stack = (dword*)(_frame->kernel_esp + 0x14); //0x14 = 20 = 5 * 4 dwords (cs, eip, eflags, int, error)
	}

	switch(service)
	{
		//Memory
		case IDX_XKY_ADDRESS_SPACE_Alloc:
		{
			_frame->eax = XKY_ADDRESS_SPACE_Alloc();
			return false;
		}
		case IDX_XKY_ADDRESS_SPACE_Free:
		{
			XKY_ADDRESS_SPACE_Free((ADDRESS_SPACE)stack[0]);
			return false;
		}
		case IDX_XKY_ADDRESS_SPACE_GetCurrent:
		{
			_frame->eax = XKY_ADDRESS_SPACE_GetCurrent();
			return false;
		}
		case IDX_XKY_ADDRESS_SPACE_SwitchTo:
		{
			_frame->eax = XKY_ADDRESS_SPACE_SwitchTo((ADDRESS_SPACE)stack[0]);
			return false;
		}
		case IDX_XKY_PAGE_Alloc:
		{
			_frame->eax = XKY_PAGE_Alloc((ADDRESS_SPACE)stack[0], (VIRTUAL)stack[1], stack[2]);
			return false;
		}
		case IDX_XKY_PAGE_Free:
		{
			XKY_PAGE_Free((ADDRESS_SPACE)stack[0], (VIRTUAL)stack[1], stack[2]);
			return false;
		}
		case IDX_XKY_PAGE_Share:
		{
			_frame->eax = XKY_PAGE_Share((ADDRESS_SPACE)stack[0], (VIRTUAL)stack[1], (ADDRESS_SPACE)stack[2], (VIRTUAL)stack[3], stack[4]);
			return false;
		}

		//Disk
		case IDX_XKY_DISK_Alloc:
		{
			_frame->eax = XKY_DISK_Alloc((LBA)stack[0], stack[1]);
			return false;
		}
		case IDX_XKY_DISK_Free:
		{
			XKY_DISK_Free((LBA)stack[0], stack[1]);
			return false;
		}
		case IDX_XKY_DISK_Read:
		{
			_frame->eax = XKY_DISK_Read((LBA)stack[0], (VIRTUAL)stack[1], stack[2]);
			return false;
		}
		case IDX_XKY_DISK_Write:
		{
			_frame->eax = XKY_DISK_Write((LBA)stack[0], (VIRTUAL)stack[1], stack[2]);
			return false;
		}

		//Windows
		case IDX_XKY_WINDOW_Alloc:
		{
			_frame->eax = XKY_WINDOW_Alloc();
			return false;
		}
		case IDX_XKY_WINDOW_Free:
		{
			XKY_WINDOW_Free((WINDOW)stack[0]);
			return false;
		}
		case IDX_XKY_WINDOW_GetHeight:
		{
			_frame->eax = XKY_WINDOW_GetHeight((WINDOW)stack[0]);
			return false;
		}
		case IDX_XKY_WINDOW_GetWidth:
		{
			_frame->eax = XKY_WINDOW_GetWidth((WINDOW)stack[0]);
			return false;
		}
		case IDX_XKY_WINDOW_GetPixel:
		{
			//Set default result
			_frame->eax = XKY_WINDOW_GetPixel((WINDOW)stack[0], stack[1], stack[2]);
			return false;
		}
		case IDX_XKY_WINDOW_SetPixel:
		{
			XKY_WINDOW_SetPixel((WINDOW)stack[0], stack[1], stack[2], (ARGB)stack[3]);
			return false;
		}
		case IDX_XKY_WINDOW_PrintText:
		{
			XKY_WINDOW_PrintText((WINDOW)stack[0], stack[1], stack[2], (ARGB)stack[3], (string*)stack[4]);
			return false;
		}
        case IDX_XKY_WINDOW_RegisterKeyboard:
		{
			XKY_WINDOW_RegisterKeyboard((WINDOW)stack[0], (ADDRESS_SPACE)stack[1], (fWindowKeyboardCallback)stack[2]);
			return false;
		}
		case IDX_XKY_WINDOW_RegisterMouse:
		{
			XKY_WINDOW_RegisterMouse((WINDOW)stack[0], (ADDRESS_SPACE)stack[1], (fWindowMouseCallback)stack[2]);
			return false;
		}
		case IDX_XKY_POINTER_GetArrow:
		{
			_frame->eax = XKY_POINTER_GetArrow();
			return false;
		}
		case IDX_XKY_POINTER_GetClock:
		{
			_frame->eax = XKY_POINTER_GetClock();
			return false;
		}
		case IDX_XKY_WINDOW_SetPointer:
		{
			XKY_WINDOW_SetPointer((WINDOW)stack[0], (POINTER)stack[1]);
			return false;
		}
		case IDX_XKY_WINDOW_SetPointerColor:
		{
			XKY_WINDOW_SetPointerColor((WINDOW)stack[0], (ARGB)stack[1]);
			return false;
		}

		//PCI
		case IDX_XKY_PCI_Alloc:
		{
			_frame->eax = XKY_PCI_Alloc((PCI)stack[0]);
			return false;
		}
		case IDX_XKY_PCI_Free:
		{
			XKY_PCI_Free((PCI)stack[0]);
			return false;
		}
		case IDX_XKY_PCI_GetNumberOfDevices:
		{
			_frame->eax = XKY_PCI_GetNumberOfDevices();
			return false;
		}
		case IDX_XKY_PCI_GetDevice:
		{
			_frame->eax = XKY_PCI_GetDevice(stack[0]);
			return false;
		}
		case IDX_XKY_PCI_GetBusFromDirection:
		{
			_frame->eax = XKY_PCI_GetBusFromDirection((PCI)stack[0]);
			return false;
		}
		case IDX_XKY_PCI_GetDeviceFromDirection:
		{
			_frame->eax = XKY_PCI_GetDeviceFromDirection((PCI)stack[0]);
			return false;
		}
		case IDX_XKY_PCI_GetFunctionFromDirection:
		{
			_frame->eax = XKY_PCI_GetFunctionFromDirection((PCI)stack[0]);
			return false;
		}
		case IDX_XKY_PCI_ReadDword:
		{
			_frame->eax = XKY_PCI_ReadDword((PCI)stack[0], stack[1]);
			return false;
		}
		case IDX_XKY_PCI_WriteDword:
		{
			XKY_PCI_WriteDword((PCI)stack[0], stack[1], stack[2]);
			return false;
		}

		//CPU
		case IDX_XKY_CPU_FillExecution:
		{
			XKY_CPU_FillExecution((EXECUTION*)stack[0], (ADDRESS_SPACE)stack[1], (VIRTUAL)stack[2], (VIRTUAL)stack[3], (INT_InterruptFromUserMode(_frame)?true:((bool)stack[4])));
			return false;
		}
		case IDX_XKY_CPU_Alloc:
		{
			_frame->eax = XKY_CPU_Alloc((XID)stack[0], (XID)stack[1]);
			return false;
		}
		case IDX_XKY_CPU_AllocCode:
		{
			_frame->eax = XKY_CPU_AllocCode((XID)stack[0], (ADDRESS_SPACE)stack[1], (VIRTUAL)stack[2], (VIRTUAL)stack[3]);
			return false;
		}
		case IDX_XKY_CPU_Free:
		{
			XKY_CPU_Free((XID)stack[0]);
			return false;
		}
		case IDX_XKY_CPU_RegisterCallback:
		{
			XKY_CPU_RegisterCallback((ADDRESS_SPACE)stack[0], (fCpuTimerCallback)stack[1]);
			return false;
		}
		case IDX_XKY_CPU_GetCurrent:
		{
			_frame->eax = XKY_CPU_GetCurrent();
			return false;
		}

		//RTC
		case IDX_XKY_RTC_Seconds:
		{
			_frame->eax = XKY_RTC_Seconds();
			return false;
		}
		case IDX_XKY_RTC_Minutes:
		{
			_frame->eax = XKY_RTC_Minutes();
			return false;
		}
		case IDX_XKY_RTC_Hour:
		{
			_frame->eax = XKY_RTC_Hour();
			return false;
		}
		case IDX_XKY_RTC_DayOfWeek:
		{
			_frame->eax = XKY_RTC_DayOfWeek();
			return false;
		}
		case IDX_XKY_RTC_DayOfMonth:
		{
			_frame->eax = XKY_RTC_DayOfMonth();
			return false;
		}
		case IDX_XKY_RTC_Month:
		{
			_frame->eax = XKY_RTC_Month();
			return false;
		}
		case IDX_XKY_RTC_Year:
		{
			_frame->eax = XKY_RTC_Year();
			return false;
		}

		//TMR
		case IDX_XKY_TMR_GetTicks:
		{
			_frame->eax = XKY_TMR_GetTicks();
			return false;
		}

		//LDR
		case IDX_XKY_LDR_GetProcedureAddress:
		{
			_frame->eax = XKY_LDR_GetProcedureAddress((VIRTUAL)stack[0], (string*)stack[1]);
			return false;
		}
		case IDX_XKY_LDR_LoadUserModule:
		{
			_frame->eax = XKY_LDR_LoadUserModule((string*)stack[0], (ADDRESS_SPACE)stack[1], (VIRTUAL)stack[2]);
			return false;
		}
		case IDX_XKY_LDR_LoadKernelModule:
		{
			_frame->eax = XKY_LDR_LoadKernelModule((string*)stack[0]);
			return false;
		}
		case IDX_XKY_LDR_LoadFile:
		{
			_frame->eax = XKY_LDR_LoadFile((string*)stack[0], (ADDRESS_SPACE)stack[1], (VIRTUAL)stack[2]);
			return false;
		}
		case IDX_XKY_LDR_FileSize:
		{
			_frame->eax = XKY_LDR_FileSize((string*)stack[0]);
			return false;
		}

		//OS
		case IDX_XKY_OS_Start:
		{
			_frame->eax = XKY_OS_Start((string*)stack[0]);
			return false;
		}
		case IDX_XKY_OS_Finish:
		{
			XKY_OS_Finish();
			return false;
		}
		
		//EXCEPTION
		case IDX_XKY_EXCEPTION_SetHandler:
		{
			_frame->eax = XKY_EXCEPTION_SetHandler((byte)stack[0], (ADDRESS_SPACE)stack[1], (fInterruptHandler)stack[2]);
			return false;
		}
		case IDX_XKY_EXCEPTION_UnsetHandler:
		{
			XKY_EXCEPTION_UnsetHandler((byte)stack[0]);
			return false;
		}
		
		//DEBUG
		case IDX_XKY_DEBUG_Message:
		{
			XKY_DEBUG_Message((string*)stack[0], stack[1]);
			return false;
		}
		case IDX_XKY_DEBUG_Data:
		{
			XKY_DEBUG_Data((string*)stack[0], stack[1], stack[2]);
			return false;
		}

		default:
		{
			DEBUG("UNKNOWN KERNEL SERVICE REQUEST");
			break;
		}
	};
	return true;
}

/**
* @brief Redirects errors to environments if a handler is found, otherwise terminates environment.
* @param _frame [in] The interrupt frame.
* @return True for further processing, false otherwise.
*/
PRIVATE int times = 0;
PRIVATE bool INTERRUPT KERNEL_EnvironmentExceptionRedirector(IN INTERRUPT_FRAME* _frame)
{
	KERNEL_Bug(_frame);
	ENVIRONMENT* environment = ENVIRONMENT_GetCurrent();
	if(environment)
	{
		//Has handler registered?
		if(!ENVIRONMENT_InvokeHandler(environment, (byte)_frame->vector, _frame))
		{
			DEBUG_DATA("Unhandled exception! = ", _frame->vector, 0x00FF0000);
			DEBUG_DATA("KERNEL_Bug(_frame) EIP = ", _frame->eip, 0x00FF0000);
			if(_frame->eip > 0x00800000)
			{
				PTE* pte = VIRTUAL_PTE_Address(CPU_ReadCR3(), _frame->eip);
				DEBUG_DATA("PHYS EIP = ", pte->address<<12, 0x00FF0000);
			}

			times++;
			if(times == 2)
			{
				KERNEL_Bug(_frame); //Quitar!
				DEBUG_DATA("KERNEL_Bug(_frame) EIP = ", _frame->eip, 0x00FF0000);
				DEBUG_DATA("KERNEL_Bug(_frame) CR2 = ", CPU_ReadCR2(), 0x00FF0000);
				__asm cli
				__asm hlt
			}

			//Exception wasn't handled or environment hasn't registered one
			//In both cases we terminate the environment
			//Free environment
			ENVIRONMENT_Release(environment);

			//Wait until a new timer interrupt changes execution
			__asm sti
			for(;;);
		}
	}
	/*
	else
	{
	//No current environment! That's impossible
	_frame->vector = 0xFFFFFFFF;
	KERNEL_Bug(_frame);
	}
	*/
	//Ok, exception was handled
	//No more processing
	return false;
}

/**
* @brief Initializes the hardware.
* @param _loader_data [in] The information the loader has compiled.
* @return True if everything get initialized, false otherwise.
*/
PRIVATE bool HARDWARE_Init(IN LOADER_DATA* _loader_data)
{
	//Screen is first as does not depend on memory and then we can show info
	if(!SVGA_Init(_loader_data->svga)) return false;
	DEBUG("HARDWARE Initializing...");
	DEBUG("  SVGA Initialized");

	//Initialize memory so other elements have dynamic memory
	if(!MEM_Init(_loader_data->memory, _loader_data->svga)) return false;
	DEBUG("  MEMORY Initialized");

	//Cpu
	if(!CPU_Init()) return false;
	DEBUG("  CPU Initialized");

	//Interrupts
	if(!INT_Init()) return false;
	DEBUG("  INTERRUPTS Initialized");

	//Put KERNEL_Bug for unhandled interrupts
	for(byte i = 0; i<32; i++)
	{
		INT_SetHandler(ExceptionInterrupt, i, KERNEL_Bug);
	}

	//PCI bus
	if(!PCI_Init()) return false;
	DEBUG("  PCI Initialized");

	//Hard disk
	if(!HD_Init(_loader_data->disk)) return false;
	DEBUG("  HARD DISK Initialized");

	//Keyboard
	if(!KBD_Init()) return false;
	DEBUG("  KEYBOARD Initialized");

	//Mouse
	if(!MOUSE_Init()) return false;
	DEBUG("  MOUSE Initialized");

	//Timer
	if(!TMR_Init()) return false;
	DEBUG("  TIMER Initialized");

	//Real-time clock
	if(!RTC_Init()) return false;
	DEBUG("  RTC Initialized");

	//Other
	//if(!DMA_Init()) return false;
	//if(!FLOPPY_Init()) return false;

	return true;
}

/**
* @brief Initializes the Kernel.
* @param _loader_data [in] The information the loader has compiled.
* @return True if everything get initialized, false otherwise.
*/
PRIVATE bool KERNEL_Init(IN LOADER_DATA* _loader_data)
{
	DEBUG("KERNEL Initializing...");

	//Runtime library support
	if(!RTL_Init(_loader_data->disk)) return false;
	DEBUG("  RUNTIME LIBRARY Initialized");

	//Windows subsystem
	if(!WINDOW_Init()) return false;

	//Debug subsystem
	if(!DEBUG_Init(TRGB(255, 255, 255))) return false;
	DEBUG("  WINDOWS SUBSYSTEM Initialized");

	//Processor support
	if(!PROCESSOR_Init()) return false;
	DEBUG("  PROCESSOR SUPPORT Initialized");

	//Address space support
	if(!ADDRESS_SPACE_Init(_loader_data->svga)) return false;
	DEBUG("  ADDRESS SPACE SUPPORT Initialized");

	//Environments support
	if(!ENVIRONMENT_Init()) return false;
	DEBUG("  ENVIRONMENT SUPPORT Initialized");

	//System services
	if(!INT_SetHandler(SoftwareInterrupt, 0x80, KERNEL_Services)) return false;
	DEBUG("  SYSTEM SERVICES Initialized");
	
	//Put KERNEL_EnvironmentExceptionRedirector for unhandled interrupts
	for(byte i = 0; i < 32; i++)
	{
		if(i != 8) //KERNEL_Bug for double fault
		{
			INT_UnsetHandler(ExceptionInterrupt, i, KERNEL_Bug);
			INT_SetHandler(ExceptionInterrupt, i, KERNEL_EnvironmentExceptionRedirector);
		}
	}
	DEBUG("  EXCEPTION HANDLERS Redirected");

	return true;
}

/**
* @brief The kernel entry-point.
* @param _loader_data [in] The information the loader has compiled.
* @return Never returns.
*/
PUBLIC void KERNEL_EntryPoint(IN LOADER_DATA* _loader_data)
{
	if(HARDWARE_Init(_loader_data))
	{
		if(KERNEL_Init(_loader_data))
		{
			if(ENVIRONMENT_Create(&CLI))
			{
				DEBUG("CLI MODULE OK!");

				//Ok, start the system
				__asm sti

				//Were done, this gets executed only until the first timer interrupt
				for(;;);
			}
			else
			{
				
				DEBUG("CLI MODULE FAILED LAUNCHING!");
			}
		}
		else
		{
			DEBUG("KERNEL INITIALIZATION FAILED!");
			__asm hlt
		}
	}
	else
	{
		DEBUG("HARDWARE INITIALIZATION FAILED!");
		__asm hlt
	}

}	

//API de KERNEL
//=================================EXPORTS====================================//
#pragma data_seg(".exports")
//============================================================================//
EXPORT(XKY_ADDRESS_SPACE_Alloc);
EXPORT(XKY_ADDRESS_SPACE_Free);
EXPORT(XKY_ADDRESS_SPACE_GetCurrent);
EXPORT(XKY_ADDRESS_SPACE_SwitchTo);
EXPORT(XKY_PAGE_Alloc);
EXPORT(XKY_PAGE_Free);
EXPORT(XKY_PAGE_Share);

EXPORT(XKY_DISK_Alloc);
EXPORT(XKY_DISK_Free);
EXPORT(XKY_DISK_Read);
EXPORT(XKY_DISK_Write);

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

EXPORT(XKY_PCI_Alloc);
EXPORT(XKY_PCI_Free);
EXPORT(XKY_PCI_GetNumberOfDevices);
EXPORT(XKY_PCI_GetDevice);
EXPORT(XKY_PCI_GetBusFromDirection);
EXPORT(XKY_PCI_GetDeviceFromDirection);
EXPORT(XKY_PCI_GetFunctionFromDirection);
EXPORT(XKY_PCI_ReadDword);
EXPORT(XKY_PCI_WriteDword);

EXPORT(XKY_CPU_FillExecution);
EXPORT(XKY_CPU_Alloc);
EXPORT(XKY_CPU_AllocCode);
EXPORT(XKY_CPU_Free);
EXPORT(XKY_CPU_RegisterCallback);
EXPORT(XKY_CPU_GetCurrent);

EXPORT(XKY_RTC_Seconds);
EXPORT(XKY_RTC_Minutes);
EXPORT(XKY_RTC_Hour);
EXPORT(XKY_RTC_DayOfWeek);
EXPORT(XKY_RTC_DayOfMonth);
EXPORT(XKY_RTC_Month);
EXPORT(XKY_RTC_Year);

EXPORT(XKY_TMR_GetTicks);

EXPORT(XKY_LDR_GetProcedureAddress);
EXPORT(XKY_LDR_LoadUserModule);
EXPORT(XKY_LDR_LoadKernelModule);
EXPORT(XKY_LDR_LoadFile);
EXPORT(XKY_LDR_FileSize);

EXPORT(XKY_OS_Start);
EXPORT(XKY_OS_Finish);

EXPORT(XKY_EXCEPTION_SetHandler);
EXPORT(XKY_EXCEPTION_UnsetHandler);
EXPORT(XKY_INTERRUPT_SetHandler);
EXPORT(XKY_INTERRUPT_UnsetHandler);

EXPORT(XKY_DEBUG_Message);
EXPORT(XKY_DEBUG_Data);

//=================================MODULE=====================================//
#pragma data_seg(".module")
//============================================================================//
	MODULE(IMAGE_MODE_SYSTEM, IMAGE_KIND_MODULE, IMAGE_VERSION(1,0,0,0), 0, KERNEL_EntryPoint, 0);
