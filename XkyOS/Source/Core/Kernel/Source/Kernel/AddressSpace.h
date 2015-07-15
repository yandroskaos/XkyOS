/******************************************************************************/
/**
* @file		AddressSpace.h
* @brief	XkyOS Virtual memory management
* Definitions of virtual memory address spaces.
* 
* @date		20/03/2008
* @author	Pablo Bravo
*/
/******************************************************************************/
#ifndef __ADDRESS_SPACE_H__
#define __ADDRESS_SPACE_H__

	#include "Types.h"
	#include "Memory.h"
	#include "System.h"

	/**
	* @brief Address space type definition.
	*/	
	typedef PHYSICAL ADDRESS_SPACE;

	bool ADDRESS_SPACE_Init(IN SVGA_LOADER_DATA* _svga_loader_data);

	ADDRESS_SPACE	ADDRESS_SPACE_Create	();
	void			ADDRESS_SPACE_Release	(IN ADDRESS_SPACE _pdbr);
	bool			ADDRESS_SPACE_Map		(IN ADDRESS_SPACE _pdbr, IN PHYSICAL _phys_address, IN VIRTUAL _virt_address, IN dword _number_of_pages, IN ExecutionType _execution, IN AccessType _access, IN bool _release);
	bool			ADDRESS_SPACE_Unmap		(IN ADDRESS_SPACE _pdbr, IN VIRTUAL _virt_address, IN dword _number_of_pages);
	bool			ADDRESS_SPACE_IsMapped	(IN ADDRESS_SPACE _pdbr, IN VIRTUAL _virt_address, IN dword _number_of_pages);

	void			ADDRESS_SPACE_SwitchTo	(IN ADDRESS_SPACE _pdbr);
	ADDRESS_SPACE	ADDRESS_SPACE_GetCurrent();

	/**
	* @brief Switches to kernel address space.
	*/	
	#define		ADDRESS_SPACE_ResetToKernelSpace() ADDRESS_SPACE_SwitchTo(KERNEL_PAGE_DIRECTORY)

#endif //__ADDRESS_SPACE_H__
