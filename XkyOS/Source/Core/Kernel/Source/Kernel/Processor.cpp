/******************************************************************************/
/**
* @file		Processor.cpp
* @brief	XkyOS Processor Slicing Library
* Definitions of processor slicing operations and types
* 
* @date		20/03/2008
* @author	Pablo Bravo
*/
/******************************************************************************/
#include "Processor.h"
#include "Interrupts.h"
#include "CPU.h"
#include "AddressSpace.h"

#include "Debug.h"
//==================================DATA======================================//
#pragma data_seg(".data")
//============================================================================//
#define MAX_EXECUTIONS	100

PRIVATE dword processor_first_time = 1;

/**
* @brief Number of executions currently running.
*/
PRIVATE dword processor_slices_used = 0;
/**
* @brief Identifies the current cpu slot.
*/
PRIVATE dword processor_current_slice = 0;
/**
* @brief Identifies the CPU as a bandwidth resource.
*/
struct PROCESSOR_SLICE
{
	bool				used;
	EXECUTION*			execution;
	fProcessorCallback	callback;
	ENVIRONMENT*		environment;
};
/**
* @brief CPU resource slices.
*/
PRIVATE PROCESSOR_SLICE processor[MAX_EXECUTIONS];

//==================================CODE======================================//
#pragma code_seg(".code")
//============================================================================//
#define TO_INDEX(X)	((X)-1)
#define TO_HANDLE(X)((X)+1)

/**
* @brief This method finds the next runnable element.
* @return Index in the CPU of the next runnable element.
*/
PRIVATE dword PROCESSOR_FindExecutionForSchedule()
{
	//Search the cpu slots vector from the next to the current element until the end...
	for(dword i = processor_current_slice + 1; i < MAX_EXECUTIONS; i++)
	{
		if(processor[i].used)
			return i;
	}
	//And then from the begining until the element before the current one...
	for(dword i = 0; i < processor_current_slice; i++)
	{
		if(processor[i].used)
			return i;
	}

	//Ok, current element is to be returned
	return processor_current_slice;
}

/**
* @brief Save the state of the task from the interrupt frame in its execution.
* @param _execution [out] Execution currently running.
* @param _frame [in] Interrupt frame with task information.
*/
PRIVATE void PROCESSOR_SaveState(OUT EXECUTION* _execution, IN INTERRUPT_FRAME* _frame)
{
	//Selectors
	_execution->gs = _frame->gs;
	_execution->fs = _frame->fs;
	_execution->es = _frame->es;
	_execution->ds = _frame->ds;
	_execution->cs = _frame->cs;
	_execution->ss = _frame->ss;
	
	//General purpose registers
	_execution->edi = _frame->edi;
	_execution->esi = _frame->esi;
	_execution->ebp = _frame->ebp;
	_execution->esp = _frame->esp;
	_execution->ebx = _frame->ebx;
	_execution->edx = _frame->edx;
	_execution->ecx = _frame->ecx;
	_execution->eax = _frame->eax;
	
	//Eip
	_execution->eip = _frame->eip;
	
	//Flags
	_execution->eflags = _frame->eflags;
	
	//PDBR
	_execution->pdbr = ADDRESS_SPACE_GetCurrent();
}

/**
* @brief Save the state of the task from the interrupt frame in its execution.
* @param _execution [in] Execution going to be scheduled.
* @param _frame [out] Interrupt frame with task information.
*/
PRIVATE void PROCESSOR_RestoreState(IN EXECUTION* _execution, OUT INTERRUPT_FRAME* _frame)
{
	//Selectors
	_frame->gs = _execution->gs;
	_frame->fs = _execution->fs;
	_frame->es = _execution->es;
	_frame->ds = _execution->ds;
	_frame->cs = _execution->cs;
	_frame->ss = _execution->ss;
	
	//General purpose registers
	_frame->edi = _execution->edi;
	_frame->esi = _execution->esi;
	_frame->ebp = _execution->ebp;
	_frame->esp = _execution->esp;
	_frame->ebx = _execution->ebx;
	_frame->edx = _execution->edx;
	_frame->ecx = _execution->ecx;
	_frame->eax = _execution->eax;
	
	//Eip
	_frame->eip = _execution->eip;
	
	//Flags
	_frame->eflags = _execution->eflags;
	
	//PDBR
	ADDRESS_SPACE_SwitchTo(_execution->pdbr);
}

/**
* @brief Processor interrupt service.
* @param _frame [in] Interrupt frame.
*/
PRIVATE bool INTERRUPT ProcessorInterrupt(IN INTERRUPT_FRAME* _frame)
{
	//if there are no tasks, do nothing...
	if(!processor_slices_used)
		return true;

	//Gain state for first execution
	if(processor_first_time)
	{
		processor_first_time = 0;
		PROCESSOR_RestoreState(processor[processor_current_slice].execution, _frame);
	}

	//Save state
	PROCESSOR_SaveState(processor[processor_current_slice].execution, _frame);
	
	//Search for the next task
	dword new_slice = PROCESSOR_FindExecutionForSchedule();
	if(new_slice != processor_current_slice)
	{
		//Change task
		processor_current_slice = new_slice;

		//Change state
		PROCESSOR_RestoreState(processor[processor_current_slice].execution, _frame);
	}

	//Call callback if there is any
	if(processor[processor_current_slice].callback)
	{
		if(processor[processor_current_slice].callback(processor_current_slice, processor[processor_current_slice].execution, processor[processor_current_slice].environment))
		{
			//Change state
			PROCESSOR_RestoreState(processor[processor_current_slice].execution, _frame);
		}

	}

	//Allow to continue the chain
	return true;
}


/**
* @brief Processor initialization.
* @return Returns true if everything goes well.
*/
PUBLIC bool PROCESSOR_Init()
{
	//Prepare slices
	for(dword i = 0; i < MAX_EXECUTIONS; i++)
	{
		processor[i].used = false;
		processor[i].execution = 0;
		processor[i].callback = 0;
		processor[i].environment = 0;
	}

	DEBUG_DATA("ProcessorInterrupt = ", (dword)ProcessorInterrupt, 0x0000FF00)
	//We set the interrupt for the cpu (on the timer)
	return INT_SetHandler(HardwareInterrupt, 0, ProcessorInterrupt);
}

/**
* @brief This method allocates an empty slot and creates an execution that will run on it.
* @param _desired_xid [in] The xid we want, or XID_ANY if doesnt mind.
* @param _execution [in] The state of the task.
* @param _environment [in] The environment associated.
* @return The xid (eXecution ID) newly allocated.
*/
PUBLIC XID PROCESSOR_CreateNewExecution(IN XID _desired_xid, IN EXECUTION* _execution, IN ENVIRONMENT* _environment)
{
	if(_desired_xid == XID_ANY)
	{
		//Search in the vector of executions
		for(dword i = 0; i < MAX_EXECUTIONS; i++)
		{
			//If it is empty...
			if(!processor[i].used)
			{
				_desired_xid = i;
				break;
			}
		}
		if(_desired_xid == XID_ANY)
		{
			//No empty room found
			return 0;
		}
		_desired_xid = TO_HANDLE(_desired_xid);
	}

	//If it is empty...
	if(!processor[TO_INDEX(_desired_xid)].used)
	{
		//Allocate
		processor[TO_INDEX(_desired_xid)].used = true;

		//Assign the slice
		processor[TO_INDEX(_desired_xid)].execution = _execution;
		processor[TO_INDEX(_desired_xid)].environment = _environment;
		processor[TO_INDEX(_desired_xid)].callback = 0;

		//Increase the number of executions
		processor_slices_used++;

		//Return XID
		return _desired_xid;
	}

	//Couldn't succeed
	return 0;
}

/**
* @brief This method allocates an empty slot for an existing execution.
* @param _desired_xid [in] The xid we would like.
* @param _xid [in] The xid of the execution we want to give more cpu.
* @return The xid (eXecution ID) newly allocated.
*/
PUBLIC XID PROCESSOR_AssignNewExecution(IN XID _desired_xid, IN XID _xid)
{
	if(_desired_xid == XID_ANY)
	{
		//Search the executions' vector
		for(dword i = 0; i < MAX_EXECUTIONS; i++)
		{
			//If it's empty...
			if(!processor[i].used)
			{
				_desired_xid = i;
				break;
			}
		}
		if(_desired_xid == XID_ANY)
		{
			//No empty room found
			return 0;
		}
		_desired_xid = TO_HANDLE(_desired_xid);
	}
	//If it's empty...
	if(!processor[TO_INDEX(_desired_xid)].used)
	{
		//Allocate
		processor[TO_INDEX(_desired_xid)].used = true;

		//Copy
		processor[TO_INDEX(_desired_xid)].execution = processor[TO_INDEX(_xid)].execution;
		processor[TO_INDEX(_desired_xid)].environment = processor[TO_INDEX(_xid)].environment;
		processor[TO_INDEX(_desired_xid)].callback = processor[TO_INDEX(_xid)].callback;

		//Increase the number of executions
		processor_slices_used++;

		//Return the XID
		return _desired_xid;
	}
	return 0;
}

/**
* @brief This method deallocates an used xid. If the execution only used that xid, it is also deleted.
* @param _xid [in] The XID of the execution we want to delete.
*/
PUBLIC void PROCESSOR_DeleteExecution(IN XID _xid)
{
	if(TO_INDEX(_xid) < MAX_EXECUTIONS && processor[TO_INDEX(_xid)].used)
	{
		//Free the slot
		processor[TO_INDEX(_xid)].used = false;
		
		//Decrease the number of executions
		processor_slices_used--;

		//Let's see if we need to free also the execution
		dword i = 0;
		for(; i < MAX_EXECUTIONS; i++)
		{
			if(i!=TO_INDEX(_xid) && processor[i].execution == processor[TO_INDEX(_xid)].execution)
				break;
		}
		if(i == MAX_EXECUTIONS)
		{
			//It was the only execution... delete
			DEBUG("CPU: Freeing Execution");
			CPU_FreeExecution(processor[TO_INDEX(_xid)].execution);
		}

		//Zero fields
		processor[TO_INDEX(_xid)].execution = 0;
		processor[TO_INDEX(_xid)].environment = 0;
		processor[TO_INDEX(_xid)].callback = 0;
	}
}

/**
* @brief Tells what the current xid is.
* @return The XID currently running.
*/
PUBLIC XID PROCESSOR_GetCurrentXID()
{
	return TO_HANDLE(processor_current_slice);
}

/**
* @brief Set the callback associated with the execution.
* @param _xid [in] The xid we want.
* @param _callback [in] The callback we want to associate.
*/
PUBLIC void PROCESSOR_RegisterCallback(IN XID _xid, IN fProcessorCallback _callback)
{
	if((TO_INDEX(_xid) < MAX_EXECUTIONS) && processor[TO_INDEX(_xid)].used)
	{
		processor[TO_INDEX(_xid)].callback = _callback;
	}
}
