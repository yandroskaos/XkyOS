/******************************************************************************/
/**
* @file		Interrupts.cpp
* @brief	XkyOS Hardware Interrupt Library
* Implementation of interrupt operations and types
* 
* @date		20/03/2008
* @author	Pablo Bravo
*/
/******************************************************************************/
#include "Types.h"
#include "System.h"
#include "IO.h"
#include "Interrupts.h"

//==================================DATA======================================//
#pragma data_seg(".data")
//============================================================================//

#define IDT_ELEMENTS	256 /**< Number of interrupts*/
#define MAX_HANDLERS_PER_INTERRUPT 4 /**< Maximum number of handlers per interrupts*/

ALIGN(4)
/**
* @brief Storage of interrupt handlers.
*/
PRIVATE fInterruptHandler int_handlers[IDT_ELEMENTS][MAX_HANDLERS_PER_INTERRUPT];

//==================================CODE======================================//
#pragma code_seg(".code")
//============================================================================//

/**
* @brief Reprograms the pic so hardware interrupts starts at vector 0x20.
*/
PRIVATE void INT_ReprogramPIC()
{
	//ICW1
	IO_OutPortByte(0x20, 0x11);
	IO_OutPortByte(0xA0, 0x11);

	//ICW2
	IO_OutPortByte(0x21, 0x20);
	IO_OutPortByte(0xA1, 0x28);

	//ICW3
	IO_OutPortByte(0x21, 0x04);
	IO_OutPortByte(0xA1, 0x02);

	//ICW4
	IO_OutPortByte(0x21, 0x01);
	IO_OutPortByte(0xA1, 0x01);

	//Program
	IO_OutPortByte(0x21, 0x00);
	IO_OutPortByte(0xA1, 0x00);
}

/**
* @brief Generic interrupt handler hooked in the IDT. Will call the handlers suscripted.
* @param [in] _frame The interrupt frame.
*/
PRIVATE void __cdecl INT_InterruptHandler(IN INTERRUPT_FRAME _frame)
{
	for(dword i = 0; i < MAX_HANDLERS_PER_INTERRUPT; i++)
	{
		if(int_handlers[_frame.vector][i])
		{
			if(!int_handlers[_frame.vector][i](&_frame))
				break;
		}
	}

	//Reset the PIC
	if(_frame.vector >= 32 && _frame.vector < 40)
	{
		//Master
		IO_OutPortByte(0x20, 0x20);
	}
	if(_frame.vector >= 40 && _frame.vector < 48)
	{
		//Slave
		IO_OutPortByte(0xA0, 0x20);
		IO_OutPortByte(0x20, 0x20);
	}
}

/**
* @brief Generic stub handler (almost) hooked in IDT. Will call the handler that will call the handlers suscripted.
*/
PRIVATE NAKED void INT_StubHandler()
{
	//Save the state
	__asm
	{
		//General purpose registers
		pushad
		//Segment registers
		push ds
		push es
		push fs
		push gs
	}

	//Call handlers
	__asm call INT_InterruptHandler

	//End Of Interrupt
	__asm
	{
		//Restore segment registers
		pop gs
		pop fs
		pop es
		pop ds
		//Restore general purpose registers
		popad
		//Pop the error and the vector
		add esp, 8
		//Back to interrupted code
		iretd
	}
}

/**
* @brief Generic interrupt handler hooked in the IDT. Will call the stub handler.
*/
#define STUB_HANDLER(X) PRIVATE NAKED void INT_StubHandler##X() \
{ \
	__asm push 0 \
	__asm push X \
	__asm jmp INT_StubHandler \
}

/**
* @brief Generic interrupt handler with error hooked in the IDT. Will call the stub handler.
*/
#define STUB_HANDLER_WITH_ERROR(X) PRIVATE NAKED void INT_StubHandlerWithError##X() \
{ \
	__asm push X \
	__asm jmp INT_StubHandler \
}

//Exceptions
STUB_HANDLER(0)
STUB_HANDLER(1)
STUB_HANDLER(2)
STUB_HANDLER(3)
STUB_HANDLER(4)
STUB_HANDLER(5)
STUB_HANDLER(6)
STUB_HANDLER(7)
STUB_HANDLER_WITH_ERROR(8)
STUB_HANDLER(9)
STUB_HANDLER_WITH_ERROR(10)
STUB_HANDLER_WITH_ERROR(11)
STUB_HANDLER_WITH_ERROR(12)
STUB_HANDLER_WITH_ERROR(13)
STUB_HANDLER_WITH_ERROR(14)
STUB_HANDLER(15)
STUB_HANDLER(16)
STUB_HANDLER(17)
STUB_HANDLER(18)
STUB_HANDLER(19)
STUB_HANDLER(20)
STUB_HANDLER(21)
STUB_HANDLER(22)
STUB_HANDLER(23)
STUB_HANDLER(24)
STUB_HANDLER(25)
STUB_HANDLER(26)
STUB_HANDLER(27)
STUB_HANDLER(28)
STUB_HANDLER(29)
STUB_HANDLER(30)
STUB_HANDLER(31)

//Hardware
STUB_HANDLER(32)
STUB_HANDLER(33)
STUB_HANDLER(34)
STUB_HANDLER(35)
STUB_HANDLER(36)
STUB_HANDLER(37)
STUB_HANDLER(38)
STUB_HANDLER(39)
STUB_HANDLER(40)
STUB_HANDLER(41)
STUB_HANDLER(42)
STUB_HANDLER(43)
STUB_HANDLER(44)
STUB_HANDLER(45)
STUB_HANDLER(46)
STUB_HANDLER(47)

//User
STUB_HANDLER(64)
STUB_HANDLER(128)
STUB_HANDLER(160)
STUB_HANDLER(192)

/**
* @brief Fills an IDT entry with a handler value.
*/
PRIVATE void INT_SetIDTHandler(IN dword _index, IN dword _handler)
{
#define IDT_MASK_LOW	0x00080000
#define IDT_MASK_HIGH	0x0000EE00
#define IDT_DIRECTION	0x00014000	//IDT Base Address

	*(dword*)(IDT_DIRECTION + _index*8) = ((_handler & 0x0000FFFF) | IDT_MASK_LOW);
	*(dword*)(IDT_DIRECTION + _index*8 + 4) = ((_handler & 0xFFFF0000) | IDT_MASK_HIGH);
}

/**
* @brief Fills the IDT with adecuate default handlers so the hook-subscription handlers procedure works.
*/
PRIVATE void INT_SetStubs()
{
	//User
	INT_SetIDTHandler(0, (dword)INT_StubHandler0);
	INT_SetIDTHandler(1, (dword)INT_StubHandler1);
	INT_SetIDTHandler(2, (dword)INT_StubHandler2);
	INT_SetIDTHandler(3, (dword)INT_StubHandler3);
	INT_SetIDTHandler(4, (dword)INT_StubHandler4);
	INT_SetIDTHandler(5, (dword)INT_StubHandler5);
	INT_SetIDTHandler(6, (dword)INT_StubHandler6);
	INT_SetIDTHandler(7, (dword)INT_StubHandler7);
	INT_SetIDTHandler(8, (dword)INT_StubHandlerWithError8);
	INT_SetIDTHandler(9, (dword)INT_StubHandler9);
	INT_SetIDTHandler(10, (dword)INT_StubHandlerWithError10);
	INT_SetIDTHandler(11, (dword)INT_StubHandlerWithError11);
	INT_SetIDTHandler(12, (dword)INT_StubHandlerWithError12);
	INT_SetIDTHandler(13, (dword)INT_StubHandlerWithError13);
	INT_SetIDTHandler(14, (dword)INT_StubHandlerWithError14);
	INT_SetIDTHandler(15, (dword)INT_StubHandler15);
	INT_SetIDTHandler(16, (dword)INT_StubHandler16);
	INT_SetIDTHandler(17, (dword)INT_StubHandler17);
	INT_SetIDTHandler(18, (dword)INT_StubHandler18);
	INT_SetIDTHandler(19, (dword)INT_StubHandler19);
	INT_SetIDTHandler(20, (dword)INT_StubHandler20);
	INT_SetIDTHandler(21, (dword)INT_StubHandler21);
	INT_SetIDTHandler(22, (dword)INT_StubHandler22);
	INT_SetIDTHandler(23, (dword)INT_StubHandler23);
	INT_SetIDTHandler(24, (dword)INT_StubHandler24);
	INT_SetIDTHandler(25, (dword)INT_StubHandler25);
	INT_SetIDTHandler(26, (dword)INT_StubHandler26);
	INT_SetIDTHandler(27, (dword)INT_StubHandler27);
	INT_SetIDTHandler(28, (dword)INT_StubHandler28);
	INT_SetIDTHandler(29, (dword)INT_StubHandler29);
	INT_SetIDTHandler(30, (dword)INT_StubHandler30);
	INT_SetIDTHandler(31, (dword)INT_StubHandler31);

	//Hardware
	INT_SetIDTHandler(32, (dword)INT_StubHandler32);
	INT_SetIDTHandler(33, (dword)INT_StubHandler33);
	INT_SetIDTHandler(34, (dword)INT_StubHandler34);
	INT_SetIDTHandler(35, (dword)INT_StubHandler35);
	INT_SetIDTHandler(36, (dword)INT_StubHandler36);
	INT_SetIDTHandler(37, (dword)INT_StubHandler37);
	INT_SetIDTHandler(38, (dword)INT_StubHandler38);
	INT_SetIDTHandler(39, (dword)INT_StubHandler39);
	INT_SetIDTHandler(40, (dword)INT_StubHandler40);
	INT_SetIDTHandler(41, (dword)INT_StubHandler41);
	INT_SetIDTHandler(42, (dword)INT_StubHandler42);
	INT_SetIDTHandler(43, (dword)INT_StubHandler43);
	INT_SetIDTHandler(44, (dword)INT_StubHandler44);
	INT_SetIDTHandler(45, (dword)INT_StubHandler45);
	INT_SetIDTHandler(46, (dword)INT_StubHandler46);
	INT_SetIDTHandler(47, (dword)INT_StubHandler47);

	//User
	INT_SetIDTHandler(64, (dword)INT_StubHandler64);
	INT_SetIDTHandler(128, (dword)INT_StubHandler128);
	INT_SetIDTHandler(160, (dword)INT_StubHandler160);
	INT_SetIDTHandler(192, (dword)INT_StubHandler192);
}

/**
* @brief Interrupt and PIC initialization.
* @return Returns true if everything goes well.
*/
PUBLIC bool INT_Init()
{
	//Fill stubs
	INT_SetStubs();

	//Reprogram the PIC
	INT_ReprogramPIC();

	return true;
}

/**
* @brief Sets a handler for a given interrupt.
* The _interrupt depends on the _kind. ExceptionInterrupt and SoftwareInterrupt are zero-based
* but HardwareInterrupt is 0x20-based.
* @param _kind [in] Type of interrupt.
* @param _interrupt [in] Number of interrupt.
* @param _handler [in] Handler for the interrupt.
* @return Returns true if handler was set.
*/
PUBLIC bool INT_SetHandler(IN InterruptKind _kind, IN byte _interrupt, IN fInterruptHandler _handler)
{
	switch(_kind)
	{
		case ExceptionInterrupt:
		{
			if(_interrupt < 32)
			{
				for(dword i = 0; i < MAX_HANDLERS_PER_INTERRUPT; i++)
				{
					if(!int_handlers[_interrupt][i])
					{
						int_handlers[_interrupt][i] = _handler;
						return true;
					}
				}
				return false;
			}
			break;
		}
		case HardwareInterrupt:
		{
			if(_interrupt < (256 - 32))
			{
				for(dword i = 0; i < MAX_HANDLERS_PER_INTERRUPT; i++)
				{
					if(!int_handlers[_interrupt + 32][i])
					{
						int_handlers[_interrupt + 32][i] = _handler;
						return true;
					}
				}
				return false;
			}
			break;
		}
		case SoftwareInterrupt:
		{
			if(_interrupt > 48)
			{
				for(dword i = 0; i < MAX_HANDLERS_PER_INTERRUPT; i++)
				{
					if(!int_handlers[_interrupt][i])
					{
						int_handlers[_interrupt][i] = _handler;
						return true;
					}
				}
				return false;
			}
			break;
		}
	}
	return false;
}

/**
* @brief Unsets a handler for a given interrupt.
* The _interrupt depends on the _kind. ExceptionInterrupt and SoftwareInterrupt are zero-based
* but HardwareInterrupt is 0x20-based.
* @param _kind [in] Type of interrupt.
* @param _interrupt [in] Number of interrupt.
* @param _handler [in] Handler for the interrupt.
*/
PUBLIC void INT_UnsetHandler(IN InterruptKind _kind, IN byte _interrupt, IN fInterruptHandler _handler)
{
	switch(_kind)
	{
		case ExceptionInterrupt:
		{
			if(_interrupt < 32)
			{
				for(dword i = 0; i < MAX_HANDLERS_PER_INTERRUPT; i++)
				{
					if(int_handlers[_interrupt][i] == _handler)
					{
						int_handlers[_interrupt][i] = 0;
						return;
					}
				}
			}
			break;
		}
		case HardwareInterrupt:
		{
			if(_interrupt < 256 - 32)
			{
				for(dword i = 0; i < MAX_HANDLERS_PER_INTERRUPT; i++)
				{
					if(int_handlers[_interrupt + 32][i] == _handler)
					{
						int_handlers[_interrupt + 32][i] = 0;
						return;
					}
				}
			}
			break;
		}
		case SoftwareInterrupt:
		{
			if(_interrupt > 48)
			{
				for(dword i = 0; i < MAX_HANDLERS_PER_INTERRUPT; i++)
				{
					if(int_handlers[_interrupt][i] == _handler)
					{
						int_handlers[_interrupt][i] = 0;
						return;
					}
				}
			}
			break;
		}
	}
}

/**
* @brief Disables interrupts.
* This functions disables interrupts. Should be matched with a call to INT_EnableInterrupts.
* @return Returns true if interrupts were set, false if not. The value should be used when calling INT_EnableInterrupts.
*/
PUBLIC dword NAKED INT_DisableInterrupts()
{
	#define IF_FLAG 0x00000200
	__asm
	{
		//Move eflags to eax
		pushfd
		pop eax
		//Test if interrupts are set
		and eax, IF_FLAG
		jz _Done
		//If set, reset
		cli
		mov eax, 0x00000001
		ret
_Done:
		mov eax, 0x00000000
		ret
	}
}

/**
* @brief Enables interrupts.
* This functions enables interrupts depending on the parameter.
*/
PUBLIC void	NAKED INT_EnableInterrupts(IN dword _state)
{
	__asm
	{
		push ebp
		mov ebp, esp
		mov eax, [ebp + 8]
		test eax, eax
		jz _Done
		sti
_Done:
		pop ebp
		ret
	}
}

/**
* @brief Indicates if an interrupt was generated from ring 3.
*/
PUBLIC bool INT_InterruptFromUserMode(IN INTERRUPT_FRAME* _frame)
{
	return _frame->cs == CODE_R3_SELECTOR;
}

/**
* @brief Indicates if an interrupt was generated from ring 3.
*/
PUBLIC bool INT_InterruptFromKernelMode(IN INTERRUPT_FRAME* _frame)
{
	return _frame->cs == CODE_R0_SELECTOR;
}

/**
* @brief Mask an interrupt, enabling or disabling it in the PIC.
* @param _interrupt [in] Interrupt number.
* @param _enable [in] Allows or denies interrupt.
*/
PRIVATE void INT_MaskHardwareInterrupts(IN byte _interrupt, IN bool _enable)
{
	byte data = 0;

	if(_interrupt <= 0x07)
	{
		byte interrupt_mask = (0x01 << _interrupt);

		//Get the current mask value from master
		data = IO_InPortByte(0x21);

		//An enabled interrupt has its mask bit off
		if(_enable)
			data &= ~interrupt_mask;
		else
			data |= interrupt_mask;

		IO_OutPortByte(0x21, data);
	}
	else
	{
		byte interrupt_mask = (0x01 << (_interrupt - 0x08));

		//Get the current mask value from slave
		data = IO_InPortByte(0xA1);

		//An enabled interrupt has its mask bit off
		if(_enable)
			data &= ~interrupt_mask;
		else
			data |= interrupt_mask;

		IO_OutPortByte(0xA1, data);
	}
}

/**
* @brief Disables all hardware interrupts.
*/
PUBLIC void INT_DisableHardwareInterrupts()
{
	for(byte i = 0; i < 15; i++)
		INT_MaskHardwareInterrupts(i, false);
}

/**
* @brief Enables all hardware interrupts.
*/
PUBLIC void INT_EnableHardwareInterrupts()
{
	for(byte i = 0; i < 15; i++)
		INT_MaskHardwareInterrupts(i, true);
}
