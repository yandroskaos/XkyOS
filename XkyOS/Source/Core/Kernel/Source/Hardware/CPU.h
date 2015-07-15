/******************************************************************************/
/**
* @file		CPU.h
* @brief	XkyOS Hardware CPU Library
* Definitions of CPU operations and types
* 
* @date		20/03/2008
* @author	Pablo Bravo
*/
/******************************************************************************/
#ifndef __CPU_H__
#define __CPU_H__

	#include "System.h"
	#include "Memory.h"
	/**
	* @brief Represents a CPU resource.
	*/
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
		//Address space
		dword pdbr;
	};

	bool	CPU_Init();

	EXECUTION*	CPU_AllocExecution();
	void		CPU_FreeExecution(IN EXECUTION* _execution);
	void		CPU_FillExecution(IN EXECUTION* _execution, IN PHYSICAL _pdbr, IN VIRTUAL _code, IN VIRTUAL _stack, IN ExecutionType _execution_mode);

	//Registers access
	dword	CPU_ReadCR0();
	void	CPU_WriteCR0(IN dword _cr0);

	dword	CPU_ReadCR2();

	dword	CPU_ReadCR3();
	void	CPU_WriteCR3(IN dword _cr3);
	
#endif //__CPU_H__
