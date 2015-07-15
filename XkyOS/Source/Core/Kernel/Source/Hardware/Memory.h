/******************************************************************************/
/**
* @file		Memory.h
* @brief	XkyOS Hardware Memory Library
* Definitions of memory operations and types
* 
* @date		20/03/2008
* @author	Pablo Bravo
*/
/******************************************************************************/
#ifndef __MEMORY_H__
#define __MEMORY_H__

	#include "System.h"
	#include "Loader.h"

	/**
	* @brief Definition for virtual addresses.
	*/
	typedef dword VIRTUAL;
	/**
	* @brief Definition for physical addresses.
	*/
	typedef dword PHYSICAL;


	bool		MEM_Init			(IN MEMORY_LOADER_DATA* _loader_data, IN SVGA_LOADER_DATA* _svga_loader_data);

	PHYSICAL	MEM_AllocPages		(IN dword _number_of_pages, IN ExecutionType _execution);
	void		MEM_ReleasePages	(IN PHYSICAL _address, IN dword _number_of_pages);

	/**
	* @brief Page directory index given a virtual address.
	*/
	#define VIRTUAL_PageDirectoryIndex(_address)	(((_address) & 0xFFC00000) >> 22)	//dword VIRTUAL_PageDirectoryIndex	(IN VIRTUAL _address);
	/**
	* @brief Page table index given a virtual address.
	*/
	#define VIRTUAL_PageTableIndex(_address)		(((_address) & 0x003FF000) >> 12)	//dword VIRTUAL_PageTableIndex		(IN VIRTUAL _address);
	/**
	* @brief Page Offset given a virtual address.
	*/
	#define VIRTUAL_PageOffset(_address)			((_address) & 0x00000FFF)			//dword VIRTUAL_PageOffset			(IN VIRTUAL _address);
	
	PDE*		VIRTUAL_PDE_Address		(IN PHYSICAL _pdbr, IN VIRTUAL _address);
	PTE*		VIRTUAL_PTE_Address		(IN PHYSICAL _pdbr, IN VIRTUAL _address);
	PHYSICAL	VIRTUAL_GetPhysical		(IN PHYSICAL _pdbr, IN VIRTUAL _address);
	
	/**
	* @brief Kernel page directory (identity mapped) address.
	*/
	#define KERNEL_PAGE_DIRECTORY		0x00401000
	
	/**
	* @brief Kernel page directory (identity mapped) address.
	*/
	#define KERNEL_PAGETABLE			0x00400000

#endif //__MEMORY_H__
