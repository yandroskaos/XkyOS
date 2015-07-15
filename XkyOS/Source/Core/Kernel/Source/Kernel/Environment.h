/******************************************************************************/
/**
* @file		Environment.h
* @brief	XkyOS Environments definitions
* Definitions of the running elements in XkyOS, which have account information
* 
* @date		20/03/2008
* @author	Pablo Bravo
*/
/******************************************************************************/
#ifndef __ENVIRONMENT_H__
#define __ENVIRONMENT_H__

	#include "Types.h"
	#include "Interrupts.h"
	#include "Processor.h"
	#include "AddressSpace.h"
	#include "DiskRange.h"
	#include "Windows.h"
	#include "PCI.h"
	
	/**
	* @brief The environment structure.
	*/
	struct ENVIRONMENT;

	/**
	* @brief The timer callback.
	*/
	typedef bool (*fCpuTimerCallback)(IN XID _xid, IN EXECUTION* _execution);

	bool ENVIRONMENT_Init();
	
	#define MODULE_START_DIRECTION	0x80000000 /**< Address where module gets mapped on new execution */
	#define STACK_START_DIRECTION	0xC0000000 /**< Execution stack */
	#define API_START_DIRECTION		0x08000000 /**< Address where api module gets mapped */
	#define NUMBER_OF_STACK_PAGES	4	/**< Number of pages allocated for initial stack */

	ENVIRONMENT*	ENVIRONMENT_Create		(IN string* _module);
	void			ENVIRONMENT_Release		(IN ENVIRONMENT* _enviroment);

	bool ENVIRONMENT_AllocPDBR	(IN ENVIRONMENT* _environment, IN ADDRESS_SPACE _address_space);
	bool ENVIRONMENT_OwnsPDBR	(IN ENVIRONMENT* _environment, IN ADDRESS_SPACE _address_space);
	bool ENVIRONMENT_FreePDBR	(IN ENVIRONMENT* _environment, IN ADDRESS_SPACE _address_space);

	bool ENVIRONMENT_AllocDISK	(IN ENVIRONMENT* _environment, IN DISK_RANGE _disk_range);
	bool ENVIRONMENT_OwnsDISK	(IN ENVIRONMENT* _environment, IN DISK_RANGE _disk_range);
	bool ENVIRONMENT_FreeDISK	(IN ENVIRONMENT* _environment, IN LBA _start_lba);

	bool ENVIRONMENT_AllocCPU	(IN ENVIRONMENT* _environment, IN XID _xid);
	bool ENVIRONMENT_OwnsCPU	(IN ENVIRONMENT* _environment, IN XID _xid);
	bool ENVIRONMENT_FreeCPU	(IN ENVIRONMENT* _environment, IN XID _xid);

	bool ENVIRONMENT_AllocWINDOW(IN ENVIRONMENT* _environment, IN WINDOW _window);
	bool ENVIRONMENT_OwnsWINDOW	(IN ENVIRONMENT* _environment, IN WINDOW _window);
	bool ENVIRONMENT_FreeWINDOW	(IN ENVIRONMENT* _environment, IN WINDOW _window);

	bool ENVIRONMENT_AllocPCI	(IN ENVIRONMENT* _environment, IN PCI _pci);
	bool ENVIRONMENT_OwnsPCI	(IN ENVIRONMENT* _environment, IN PCI _pci);
	bool ENVIRONMENT_FreePCI	(IN ENVIRONMENT* _environment, IN PCI _pci);


	ENVIRONMENT* ENVIRONMENT_GetCurrent	();
	ENVIRONMENT* ENVIRONMENT_First		();
	ENVIRONMENT* ENVIRONMENT_Next		(IN ENVIRONMENT* _last_environment);

	void	ENVIRONMENT_SetTimerCallback	(IN ENVIRONMENT* _environment, IN ADDRESS_SPACE _pdbr, IN fCpuTimerCallback _callback);
	void	ENVIRONMENT_UnsetTimerCallback	(IN ENVIRONMENT* _environment);

	bool	ENVIRONMENT_SetExceptionHandler		(IN ENVIRONMENT* _environment, IN byte _exception, IN ADDRESS_SPACE _pdbr, IN fInterruptHandler _handler);
	void	ENVIRONMENT_UnsetExceptionHandler	(IN ENVIRONMENT* _environment, IN byte _exception);
	bool	ENVIRONMENT_InvokeHandler			(IN ENVIRONMENT* _environment, IN byte _exception, IN INTERRUPT_FRAME* _frame);


#endif //__ENVIRONMENT_H__
