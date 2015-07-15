/******************************************************************************/
/**
* @file		CPU.cpp
* @brief	XkyOS Hardware CPU Library
* Implementation of CPU operations and types
* 
* @date		20/03/2008
* @author	Pablo Bravo
*/
/******************************************************************************/
#include "Types.h"
#include "CPU.h"

#include "Debug.h"
//==================================DATA======================================//
#pragma data_seg(".data")
//============================================================================//
#define INITIAL_EFLAGS	0x00000202
#define MAX_EXECUTIONS	100

/**
* @brief Represents a block in the 'heap' of executions.
*/
struct HEAP_EXECUTION_BLOCK
{
	bool used;
	EXECUTION execution;
};

/**
* @brief Executions Heap.
*/
PRIVATE HEAP_EXECUTION_BLOCK executions_heap[MAX_EXECUTIONS];

//==================================CODE======================================//
#pragma code_seg(".code")
//============================================================================//

/**
* @brief This method allocates an empty execution.
* @return An execution to be used.
*/
PUBLIC EXECUTION* CPU_AllocExecution()
{
	for(dword i = 0; i < MAX_EXECUTIONS; i++)
	{
		if(!executions_heap[i].used)
		{
			executions_heap[i].used = true;
			return &executions_heap[i].execution;
		}
	}
	return 0;
}

/**
* @brief This method frees a used execution.
* @param _execution [in] Execution to be freed.
*/
PUBLIC void CPU_FreeExecution(IN EXECUTION* _execution)
{
	for(dword i = 0; i < MAX_EXECUTIONS; i++)
	{
		if(&executions_heap[i].execution == _execution)
		{
			executions_heap[i].used = false;
			return;
		}
	}
}

/**
* @brief CPU initialization.
* @return Returns true if everything goes well.
*/
PUBLIC bool CPU_Init()
{
	//Initialize the 'heap'
	for(dword i = 0; i < MAX_EXECUTIONS; i++)
	{
		executions_heap[i].used = false;
	}

	return true;
}

/**
* @brief This method fills with an initial state the values of a cpu execution.
* @param _execution [in] The cpu execution containing the task state.
* @param _pdbr [in] The PDBR for the task.
* @param _code [in] The code the task will start executing.
* @param _stack [in] The stack for the task.
* @param _execution_mode [in] The starting mode of execution (Kernel or User).
*/
PUBLIC void CPU_FillExecution(IN EXECUTION* _execution, IN PHYSICAL _pdbr, IN VIRTUAL _code, IN VIRTUAL _stack, IN ExecutionType _execution_mode)
{
	//Segment selectors
	if(_execution_mode == UserMode)
	{
		_execution->cs = CODE_R3_SELECTOR;
		_execution->ds = DATA_R3_SELECTOR;
		_execution->es = DATA_R3_SELECTOR;
		_execution->fs = DATA_R3_SELECTOR;
		_execution->gs = DATA_R3_SELECTOR;
		_execution->ss = DATA_R3_SELECTOR;
	}
	else
	{
		_execution->cs = CODE_R0_SELECTOR;
		_execution->ds = DATA_R0_SELECTOR;
		_execution->es = DATA_R0_SELECTOR;
		_execution->fs = DATA_R0_SELECTOR;
		_execution->gs = DATA_R0_SELECTOR;
		_execution->ss = DATA_R0_SELECTOR;
	}

	//General purpose registers
	_execution->eax = 0;
	_execution->ecx = 0;
	_execution->edx = 0;
	_execution->ebx = 0;
	_execution->esp = _stack;
	_execution->ebp = 0;
	_execution->esi = 0;
	_execution->edi = 0;

	//Eip
	_execution->eip = _code;

	//Flags
	_execution->eflags = INITIAL_EFLAGS;

	//PDBR
	_execution->pdbr = _pdbr;
}

/**
* @brief Reads CR0.
* @return The value of CR0.
*/
PUBLIC NAKED dword CPU_ReadCR0()
{
	__asm
	{
		mov eax, cr0
		ret
	}
}

/**
* @brief Writes in CR0.
* @param _cr0 [in] The new value of CR0.
*/
PUBLIC NAKED void CPU_WriteCR0(IN dword _cr0)
{
	__asm
	{
		push eax
		mov eax, [esp + 8]
		mov cr0, eax
		pop eax
		ret 4
	}
}

/**
* @brief Reads CR2.
* @return The value of CR2 (address that caused the last 0x0E exception).
*/
PUBLIC NAKED dword CPU_ReadCR2()
{
	__asm
	{
		mov eax, cr2
		ret
	}
}

/**
* @brief Reads CR3.
* @return The value of CR3.
*/
PUBLIC NAKED dword CPU_ReadCR3()
{
	__asm
	{
		mov eax, cr3
		ret
	}
}

/**
* @brief Writes in CR3.
* @param _cr3 [in] The new value of the PDBR.
*/
PUBLIC NAKED void CPU_WriteCR3(IN dword _cr3)
{
	__asm
	{
		push eax
		mov eax, [esp + 8]
		mov cr3, eax
		pop eax
		ret 4
	}
}
