/******************************************************************************/
/**
* @file		Memory.cpp
* @brief	XkyOS Hardware Memory Library
* Implementation of memory operations and types
* 
* @date		20/03/2008
* @author	Pablo Bravo
*/
/******************************************************************************/

#include "Types.h"
#include "CPU.h"
#include "Memory.h"

//==================================DATA======================================//
#pragma data_seg(".data")
//============================================================================//

#define PAGE_TABLES_START	0x00400000	/**< Comienzo de los 4MB de page entries */
#define PAGE_TABLES_END		0x00800000	/**< Fin de los 4MB de page entries */

/**
* @brief Defines a range of memory based on it's page tables.
*/
struct MEMORY_RANGE
{
	PageEntry* start;
	PageEntry* end;
};

ALIGN(4)
/**
* @brief Kernel physical memory page tables.
*/
PRIVATE MEMORY_RANGE mem_kernel_range =
{
	(PE*)(PAGE_TABLES_START + PAGE_SIZE/4),
	(PE*)(PAGE_TABLES_START + PAGE_SIZE)
};

ALIGN(4)
/**
* @brief User physical memory page tables.
*/
PRIVATE MEMORY_RANGE mem_user_range =
{
	(PageEntry*)(PAGE_TABLES_START + 2*PAGE_SIZE),
	(PageEntry*)(PAGE_TABLES_END)
};

//==================================CODE======================================//
#pragma code_seg(".code")
//============================================================================//
/**
* @brief Memory allocation.
* @param _range [in] A memory range (represented by it's page tables).
* @param _number_of_pages [in] Number of contiguos pages to reserve.
* @return Returns first page entry if enough pages are found.
*/
PRIVATE PE* MR_Find(IN MEMORY_RANGE* _range, IN dword _number_of_pages)
{
	for(PE* pe = _range->start; pe < _range->end; )
	{
		//Find _number_of_pages
		dword i;
		for(i = 0; i < _number_of_pages; i++)
		{
			if(!pe[i].pte.present || !pe[i].pte.available)
				break;
		}
		if(i == _number_of_pages)
		{
			//Found, so do the reserve
			for(i = 0; i < _number_of_pages; i++)
				pe[i].pte.available = 0;

			return pe;
		}
		//Jump over 'i' pages
		pe += i?i:1;
	}

	return 0;
}

#define MEMORY_TYPE_UNAVAILABLE		0x00000000 /**< Indicates the memory range is inexistent */
/**
* @brief Verifies that a given physical address does exists currently.
* @param _address [in] Physical address.
* @param _loader_data [in] Loader data regarding physical memory.
* @return Returns memory type.
*/
PRIVATE dword PHYSICAL_IsValid(IN PHYSICAL _address, IN MEMORY_LOADER_DATA* _loader_data, IN SVGA_LOADER_DATA* _svga_loader_data)
{
	//Check physical memory ranges
	for(dword i = 0; i < _loader_data->descriptors_number; i++)
	{
		//Check address is in a range that is valid as physical memory
		if(
		(_address >= _loader_data->descriptors[i].base_address_low)
		&&
		(_address <= (_loader_data->descriptors[i].base_address_low + _loader_data->descriptors[i].base_length_low))
		)
			return _loader_data->descriptors[i].type;
	}

	//Check svga mapped/io ranges
	if(
		(_address >= _svga_loader_data->framebuffer)
		&&
		(_address < _svga_loader_data->framebuffer + (_svga_loader_data->x_resolution*_svga_loader_data->y_resolution)*4)
	)
		return MEMORY_TYPE_SYSTEM_RESERVED;

	//Not a valid physical address
	return MEMORY_TYPE_UNAVAILABLE;
}

/**
* @brief Memory initialization.
* @param _loader_data [in] Loader data regarding memory.
* @return Returns true if everything goes well.
*/
PUBLIC bool MEM_Init(IN MEMORY_LOADER_DATA* _memory_loader_data, SVGA_LOADER_DATA* _svga_loader_data)
{
	//We walk the memory map filling PE's
	PHYSICAL address = 0;
	for(PE* pe = (PE*)(PAGE_TABLES_START); pe < (PE*)(PAGE_TABLES_END); pe++, address+=PAGE_SIZE)
	{
		dword memory_type = PHYSICAL_IsValid(address, _memory_loader_data, _svga_loader_data);

		if(memory_type != MEMORY_TYPE_UNAVAILABLE || (address < 0x00100000))
		{
			pe->pte.present			= 1;
			pe->pte.read_write		= ReadWrite;
			pe->pte.user_supervisor	= KernelMode;
			pe->pte.write_through	= 0;
			pe->pte.cache_disabled	= 1;
			pe->pte.accesed			= 0;
			pe->pte.dirty			= 0;
			pe->pte.pat				= 0;
			pe->pte.global			= 0;
			pe->pte.available		= 1;
			pe->pte.address			= address>>12;
		}
		else
		{
			pe->pte.value = 0;
		}
	}
	//Mark as not free the first MB (the three next until four are for kernel memory)
	for(PE* pe = (PE*)PAGE_TABLES_START; pe < (PE*)(PAGE_TABLES_START+(PAGE_SIZE/4)); pe++)
	{
		pe->pte.available = 0;
	}
	
	//Activate pagination
	CPU_WriteCR3(KERNEL_PAGE_DIRECTORY);

	dword cr0 = CPU_ReadCR0();
	cr0 |= 0x80000000;		//Activate pagination
	cr0 &= 0xFFFEFFFF;		//Deactivate WP
	CPU_WriteCR0(cr0);

	//Ok
	return true;
}

/**
* @brief Memory allocation.
* @param _number_of_pages [in] Number of contiguos pages to reserve.
* @param _execution [in] Indicates execution mode (Kernel or User).
* @return Returns physical address of first requested page or zero.
*/
PUBLIC PHYSICAL MEM_AllocPages(IN dword _number_of_pages, IN ExecutionType _execution)
{
	MEMORY_RANGE* range = 0;
	switch(_execution)
	{
		case KernelMode:
		{
			range = &mem_kernel_range;
			break;
		}
		case UserMode:
		{
			range = &mem_user_range;
			break;
		}
	}
	if(range)
	{
		PE* pe = MR_Find(range, _number_of_pages);
		return pe ? ((pe->pte.address)<<12) : 0;
	}
	return 0;
}

/**
* @brief Memory deallocation.
* @param _address [in] Physical address of first of '_number_of_pages' pages previously reserved.
* @param _number_of_pages [in] Number of contiguous pages to reserve.
*/
PUBLIC void MEM_ReleasePages(IN PHYSICAL _address, IN dword _number_of_pages)
{
	for(dword i = 0; i < _number_of_pages; i++)
	{
		PTE* pte = VIRTUAL_PTE_Address(KERNEL_PAGE_DIRECTORY, _address + i*PAGE_SIZE);
		if(pte)
			pte->available = 1;
	}
}

/**
* @brief Obtain the page directory address for a virtual address
* @param _pdbr [in] Page directory base register of virtual space
* @param _address [in] Virtual address
* @return The PDE 
*/
PUBLIC PDE* VIRTUAL_PDE_Address(IN PHYSICAL _pdbr, IN VIRTUAL _address)
{
	PD* page_directory = (PD*)_pdbr;
	return &page_directory->entries[VIRTUAL_PageDirectoryIndex(_address)];
}

/**
* @brief Obtain the page table address for a virtual address.
* @param _pdbr [in] Page directory base register of virtual space.
* @param _address [in] Virtual address.
* @return The PTE if available.
*/
PUBLIC PTE* VIRTUAL_PTE_Address(IN PHYSICAL _pdbr, IN VIRTUAL _address)
{
	PDE* pde = VIRTUAL_PDE_Address(_pdbr, _address);
	if(pde->present)
	{
		PT* page_table = (PT*)(pde->address << 12);
		return &page_table->entries[VIRTUAL_PageTableIndex(_address)];
	}
	return 0;
}

/**
* @brief Obtain the physical address from the virtual address for a given addres space.
* @param _pdbr [in] Page directory base register of virtual space.
* @param _address [in] Virtual address.
* @return The physical address, or zero if not mapped.
*/
PUBLIC PHYSICAL VIRTUAL_GetPhysical(IN PHYSICAL _pdbr, IN VIRTUAL _address)
{
	PTE* pte = VIRTUAL_PTE_Address(_pdbr, _address);
	if(pte)
	{
		return pte->address << 12;
	}
	return 0;
}
