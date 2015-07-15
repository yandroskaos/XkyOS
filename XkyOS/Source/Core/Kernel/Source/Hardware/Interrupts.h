/******************************************************************************/
/**
* @file		Interrupts.h
* @brief	XkyOS Hardware Interrupt Library
* Definitions of interrupt operations and types
* 
* @date		20/03/2008
* @author	Pablo Bravo
*/
/******************************************************************************/
#ifndef __INTERRUPTS_H__
#define __INTERRUPTS_H__

	/**
	* @brief The frame with all the information when an interrupt ocurrs.
	*/
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

	#define INTERRUPT /**< Label for interrupt procedure*/

	/**
	* @brief Type for interrupts handlers.
	* @param [in] _frame The interrupt frame.
	* @return Indicates if further processing is allowed for other handlers.
	*/
	typedef bool (INTERRUPT *fInterruptHandler)(IN INTERRUPT_FRAME* _frame);

	/**
	* @brief Sources of interrupts.
	*/
	enum InterruptKind
	{
		ExceptionInterrupt = 1,	/**< Range 0 to 31*/
		HardwareInterrupt,		/**< Range 32 to 48*/
		SoftwareInterrupt		/**< Range 48 to 255*/
	};

    bool	INT_Init();
	bool	INT_SetHandler		(IN InterruptKind _kind, IN byte _interrupt, IN fInterruptHandler _handler);
	void	INT_UnsetHandler	(IN InterruptKind _kind, IN byte _interrupt, IN fInterruptHandler _handler);
	dword	INT_DisableInterrupts	();
	void	INT_EnableInterrupts	(IN dword _state);
	bool	INT_InterruptFromUserMode	(IN INTERRUPT_FRAME* _frame);
	bool	INT_InterruptFromKernelMode	(IN INTERRUPT_FRAME* _frame);
	void	INT_DisableHardwareInterrupts	();
	void	INT_EnableHardwareInterrupts	();

#endif //__INTERRUPTS_H__
