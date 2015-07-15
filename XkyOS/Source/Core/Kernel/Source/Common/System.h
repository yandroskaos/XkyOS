/******************************************************************************/
/**
* @file		System.h
* @brief	Common Intel x86 and system definitions
* Definitions of Intel x86 structures and system constants.
* 
* @date		20/03/2008
* @author	Pablo Bravo
*/
/******************************************************************************/
#ifndef __SYSTEM_H__
#define __SYSTEM_H__

	#include "Types.h"

	/**
	* @brief Definition of a page directory entry (32 bits).
	*/
	struct PageDirectoryEntry
	{
		union
		{
			/**
			* @brief Page directory entry when present.
			*/
			struct
			{
				dword present			:1;
				dword read_write		:1;
				dword user_supervisor	:1;
				dword write_through		:1;
				dword cache_disabled	:1;
				dword accesed			:1;
				dword zero				:1;
				dword size				:1;
				dword global			:1;
				//dword available		:3;
				//{
				dword available			:3;
				//}
				dword address			:20;
			};

			/**
			* @brief Page directory entry when not present.
			*/
			struct
			{
				dword present			:1;
				dword unused			:31;
			};

			/**
			* @brief Page directory entry as a whole.
			*/
			dword value;
		};
	};

	/**
	* @brief Definition of a page table entry (32 bits).
	*/
	struct PageTableEntry
	{
		union
		{
			/**
			* @brief Page table entry when present.
			*/
			struct
			{
				dword present			:1;
				dword read_write		:1;
				dword user_supervisor	:1;
				dword write_through		:1;
				dword cache_disabled	:1;
				dword accesed			:1;
				dword dirty				:1;
				dword pat				:1;
				dword global			:1;
				//dword available		:3;
				//{
				dword available			:3;
				//}
				dword address			:20;
			};

			/**
			* @brief Page table entry when present.
			*/
			struct
			{
				dword present			:1;
				dword unused			:31;
			};

			/**
			* @brief Page table entry as a whole.
			*/
			dword value;
		};
	};


	/**
	* @brief Page entry as a PDE or as a PTE.
	*/
	union PageEntry
	{
		PageDirectoryEntry	pde;
		PageTableEntry		pte;
	};

	/**
	* @brief A page directory.
	*/
	struct PageDirectory
	{
		PageDirectoryEntry entries[1024];
	};

	/**
	* @brief A page table.
	*/
	struct PageTable
	{
		PageTableEntry entries[1024];
	};


	typedef PageDirectoryEntry	PDE; /**< Shortcut for page directory entry */
	typedef PageTableEntry		PTE; /**< Shortcut for page table entry */
	typedef PageEntry			PE; /**< Shortcut for generic page entry */
	typedef PageDirectory		PD; /**< Shortcut for page directory */
	typedef PageTable			PT; /**< Shortcut for page table */

	//Constants
	/**
	* @brief System memory page size. 4KB.
	*/
	#define PAGE_SIZE	0x1000 

	/**
	* @brief System disk sector size. 512B.
	*/
	#define SECTOR_SIZE	0x0200

	//Selectors
	/**
	* @brief Selector for ring 0 (kernel) code.
	*/
	#define CODE_R0_SELECTOR		0x0008
	/**
	* @brief Selector for ring 0 (kernel) data.
	*/
	#define DATA_R0_SELECTOR		0x0010
	/**
	* @brief Selector for ring 3 (user) code.
	*/
	#define CODE_R3_SELECTOR		0x001B
	/**
	* @brief Selector for ring 3 (user) data.
	*/
	#define DATA_R3_SELECTOR		0x0023

	/**
	* @brief Kernel stack (ring 0).
	*/
	#define KERNEL_STACK	0x0000FFFC

	/**
	* @brief Execution mode.
	*/
	enum ExecutionType
	{
		KernelMode = 0,
		UserMode = 1
	};
	/**
	* @brief Page permissions.
	*/
	enum AccessType
	{
		ReadOnly = 0,
		ReadWrite = 1
	};

#endif //__SYTEM_H__
