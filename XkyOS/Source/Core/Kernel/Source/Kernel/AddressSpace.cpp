/******************************************************************************/
/**
* @file		AddressSpace.cpp
* @brief	XkyOS Virtual memory management
* Implementation of virtual memory address spaces.
* 
* @date		20/03/2008
* @author	Pablo Bravo
*/
/******************************************************************************/
#include "AddressSpace.h"
#include "CPU.h"
#include "RTL.h"

//==================================DATA======================================//
#pragma code_seg(".data")
//============================================================================//
/**
* @brief SVGA info for mapping SVGA in environments user space.
*/
PRIVATE SVGA_LOADER_DATA svga_mapping_info;

//==================================CODE======================================//
#pragma code_seg(".code")
//============================================================================//
/**
* @brief Init memory contexts subystem.
* @param _svga_loader_data [in] SVGA information for mapping in newly created address spaces.
* @return True if the subsystem was correctly initialized.
*/
PUBLIC bool ADDRESS_SPACE_Init(IN SVGA_LOADER_DATA* _svga_loader_data)
{
	//Copy mapping info
	svga_mapping_info = *_svga_loader_data;

	//Ok
	return true;
}

/**
* @brief Maps a page in a virtual memory address space.
* @param _pdbr [in] The page directory address of the address space.
* @param _phys_address [in] Physical address of page being mapped.
* @param _virt_address [in] Virtual address to map the page to.
* @param _execution [in] Permission of execution (Kernel vs User).
* @param _access [in] Kind of access allowed to page (Read only vs read and write).
* @return True if the page was mapped, false otherwise.
*/
PRIVATE bool ADDRESS_SPACE_MapPage(IN ADDRESS_SPACE _pdbr, IN PHYSICAL _phys_address, IN VIRTUAL _virt_address, IN ExecutionType _execution, IN AccessType _access, IN bool _release)
{
	PDE* pde = VIRTUAL_PDE_Address(_pdbr, _virt_address);
	if(pde->present)
	{
		PTE* pte = VIRTUAL_PTE_Address(_pdbr, _virt_address);
		if(pte->present)
		{
			//In this virtual address there is already mapped a page
			return false;
		}
		else
		{
			//Map
			pte->value = _phys_address & 0xFFFFF000;

			pte->present			= 1;
			pte->read_write			= _access;
			pte->user_supervisor	= _execution;
			pte->cache_disabled		= 1;
			pte->available			= _release;
			return true;
		}
	}
	else
	{
		//Alloc a page table for the memory range
		PT* page_table = (PT*)MEM_AllocPages(1, _execution);
		if(!page_table)
			return false;

		//Fill
		for(dword i = 0; i < PAGE_SIZE/4; i++)
		{
			//Entry as a whole
			page_table->entries[i].value = 0;
		}

		//Link the new page table
		pde->value = ((dword)page_table) & 0xFFFFF000;
		pde->present			= 1;
		pde->read_write			= _access;
		pde->user_supervisor	= _execution;
		pde->cache_disabled		= 1;

		//Now there is a page table, do map
		return ADDRESS_SPACE_MapPage(_pdbr, _phys_address, _virt_address, _execution, _access, _release);
	}
}

/**
* @brief Unmaps a virtual address page in a virtual memory address space.
* @param _pdbr [in] The page directory address of the address space.
* @param _virt_address [in] Virtual address to map the page to.
* @return True if the page was unmapped, false otherwise.
*/
PRIVATE bool ADDRESS_SPACE_UnmapPage(IN ADDRESS_SPACE _pdbr, IN VIRTUAL _virt_address)
{
	PTE* pte = VIRTUAL_PTE_Address(_pdbr, _virt_address);
	if(pte && pte->present)
	{
		//if marked for deletion...
		if(pte->available)
			MEM_ReleasePages(pte->address<<12, 1);

		//Zero all entry
		pte->value = 0;
		return true;
	}
	return false;
}

/**
* @brief Creates a virtual memory address space.
* @return The physical address of the page directory if successful, 0 otherwise.
*/
PUBLIC ADDRESS_SPACE ADDRESS_SPACE_Create()
{
	//Create memory tables
	//1 page for page directory
	//1 page for kernel (4M)
	//1 page for memory (4M)
	PHYSICAL address_space = MEM_AllocPages(3, UserMode);
	if(!address_space)
		return 0;

	PageDirectory* directory = (PageDirectory*)(address_space);
	PageTable* kernel = (PageTable*)(address_space + PAGE_SIZE);
	PageTable* memory = (PageTable*)(address_space + 2*PAGE_SIZE);

	//Copy 8K from PAGE_TABLES_START
	RTL_Copy((PHYSICAL)kernel, KERNEL_PAGETABLE, PAGE_SIZE);
	RTL_Copy((PHYSICAL)memory, KERNEL_PAGE_DIRECTORY, PAGE_SIZE);

	//Link
	directory->entries[0].value = (dword)kernel;
	directory->entries[0].present			= 1;
	directory->entries[0].read_write		= 1;
	directory->entries[0].cache_disabled	= 1;

	directory->entries[1].value = (dword)memory;
	directory->entries[1].present			= 1;
	directory->entries[1].read_write		= 1;
	directory->entries[1].cache_disabled	= 1;

	for(dword i = 2; i < PAGE_SIZE/4; i++)
	{
		directory->entries[i].value				= 0;
	}

	//Map video memory
	//ADDRESS_SPACE_Map(address_space, svga_mapping_info.framebuffer, svga_mapping_info.framebuffer, (svga_mapping_info.x_resolution*svga_mapping_info.y_resolution*4)/PAGE_SIZE, KernelMode, ReadWrite, true);
	ADDRESS_SPACE_Map(address_space, svga_mapping_info.framebuffer, svga_mapping_info.framebuffer, RTL_BytesToPages(svga_mapping_info.x_resolution*svga_mapping_info.y_resolution*4), KernelMode, ReadWrite, true);

	//Ok
	return address_space;
}

/**
* @brief Maps a range of pages in a virtual memory address space.
* @param _pdbr [in] The page directory address of the address space.
* @param _phys_address [in] Physical address of page being mapped.
* @param _virt_address [in] Virtual address to map the page to.
* @param _number_of_pages [in] Number of pages to be mapped.
* @param _execution [in] Permission of execution (Kernel vs User).
* @param _access [in] Kind of access allowed to page (Read only vs read and write).
* @param _release [in] Release de physical page when unmapping the page.
* @return True if the pages were mapped, false otherwise. If unsuccessful no page gets mapped.
*/
PUBLIC bool ADDRESS_SPACE_Map(IN ADDRESS_SPACE _pdbr, IN PHYSICAL _phys_address, IN VIRTUAL _virt_address, IN dword _number_of_pages, IN ExecutionType _execution, IN AccessType _access, IN bool _release)
{
	for(dword i = 0; i < _number_of_pages; i++)
	{
		if(!ADDRESS_SPACE_MapPage(_pdbr, _phys_address + PAGE_SIZE*i, _virt_address + PAGE_SIZE*i, _execution, _access, _release))
		{
			for(dword j = 0; j < i; j++)
			{
				ADDRESS_SPACE_UnmapPage(_pdbr, _virt_address + PAGE_SIZE*j);
			}

			return false;
		}
	}
	return true;
}

/**
* @brief Unmaps a range of pages in a virtual memory address space.
* @param _pdbr [in] The page directory address of the address space.
* @param _virt_address [in] Virtual address to map the page to.
* @param _number_of_pages [in] Number of pages to be mapped.
* @return True if the pages were unmapped, false otherwise. If unsuccessful no page gets unmapped beyond the source of error.
*/
PUBLIC bool ADDRESS_SPACE_Unmap(IN ADDRESS_SPACE _pdbr, IN VIRTUAL _virt_address, IN dword _number_of_pages)
{
	for(dword i = 0; i < _number_of_pages; i++)
	{
		if(!ADDRESS_SPACE_UnmapPage(_pdbr, _virt_address + PAGE_SIZE*i))
		{
			return false;
		}
	}
	return true;
}

/**
* @brief Tells if a given virtual address is mapped in a virtual memory address space.
* @param _pdbr [in] The page directory address of the address space.
* @param _virt_address [in] Virtual address to map the page to.
* @return True if the address is mapped, false otherwise.
*/
PRIVATE bool ADDRESS_SPACE_IsMappedPage(IN ADDRESS_SPACE _pdbr, IN VIRTUAL _virt_address)
{
	PTE* pte = VIRTUAL_PTE_Address(_pdbr, _virt_address);
	if(pte && pte->present)
	{
		return true;
	}
	return false;
}

/**
* @brief Tells if a given range of virtual addresses is mapped in a virtual memory address space.
* @param _pdbr [in] The page directory address of the address space.
* @param _virt_address [in] Virtual address to map the page to.
* @param _number_of_pages [in] Number of pages.
* @return True if the address is mapped, false otherwise.
*/
PUBLIC bool ADDRESS_SPACE_IsMapped(IN ADDRESS_SPACE _pdbr, IN VIRTUAL _virt_address, IN dword _number_of_pages)
{
	for(dword i = 0; i < _number_of_pages; i++)
	{
		if(!ADDRESS_SPACE_IsMappedPage(_pdbr, _virt_address + PAGE_SIZE*i))
		{
			return false;
		}
	}
	return true;
}

/**
* @brief Deletes a virtual memory address space.
* @param _pdbr [in] The page directory address of the address space.
*/
PUBLIC void ADDRESS_SPACE_Release(IN ADDRESS_SPACE _pdbr)
{
	PageDirectory* directory = (PageDirectory*)(_pdbr);

	for(dword i = 2; i < 1024; i++)
	{
		if(directory->entries[i].present)
		{
			PageTable* table = (PageTable*)(directory->entries[i].address << 12);
			for(dword j = 0; j < 1024; j++)
			{
				//if marked for deletion...
				if(/*table->entries[j].present &&*/ table->entries[j].available)
					MEM_ReleasePages((table->entries[j].address << 12), 1);
			}
			//Page tables are all freed...
			MEM_ReleasePages((directory->entries[i].address << 12), 1);
		}
	}

	MEM_ReleasePages(_pdbr, 3);
}

/**
* @brief Changes current address space to the one indicated.
* @param _pdbr [in] The address space we want to switch to.
*/
PUBLIC void	ADDRESS_SPACE_SwitchTo(IN ADDRESS_SPACE _pdbr)
{
	if(CPU_ReadCR3() != _pdbr)
		CPU_WriteCR3(_pdbr);
}

/**
* @brief Gets current address space.
* @return The address space we want are in.
*/
PUBLIC ADDRESS_SPACE ADDRESS_SPACE_GetCurrent()
{
	return CPU_ReadCR3();
}
