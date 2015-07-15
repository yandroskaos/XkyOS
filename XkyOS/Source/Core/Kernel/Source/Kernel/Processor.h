/******************************************************************************/
/**
* @file		Processor.h
* @brief	XkyOS Processor Slicing Library
* Definitions of processor slicing operations and types
* 
* @date		20/03/2008
* @author	Pablo Bravo
*/
/******************************************************************************/
#ifndef __PROCESSOR_H__
#define __PROCESSOR_H__

	#include "Types.h"
	#include "CPU.h"

	/**
	* @brief CPU slots resource type.
	*/
	typedef dword XID;
	#define XID_ANY 0xFFFFFFFF /**< In case we dont want an specific XID (_desired_xid)*/
	
	struct ENVIRONMENT; /**< Forwarded definition*/
	
	/**
	* @brief Callback invoked on timer interrupts.
	* @param _xid [in] The xid that is going to be executed.
	* @param _execution [in] The execution state of the xid.
	* @param _environment [in] The environment owner of the xid.
	* @return True if _execution has changed.
	*/
	typedef bool (*fProcessorCallback)(IN XID _xid, IN EXECUTION* _execution, IN ENVIRONMENT* _environment);

	bool	PROCESSOR_Init();

	XID		PROCESSOR_CreateNewExecution	(IN XID _desired_xid, IN EXECUTION* _execution, IN ENVIRONMENT* _environment);
	XID		PROCESSOR_AssignNewExecution	(IN XID _desired_xid, IN XID _xid);
	void	PROCESSOR_DeleteExecution		(IN XID _xid);
	XID		PROCESSOR_GetCurrentXID			();
	void	PROCESSOR_RegisterCallback		(IN XID _xid, IN fProcessorCallback _callback);


#endif //__PROCESSOR_H__
