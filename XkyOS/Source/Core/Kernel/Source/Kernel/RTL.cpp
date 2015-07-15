/******************************************************************************/
/**
* @file		RTL.cpp
* @brief	XkyOS Kernel Runtime Library
* Implementation of runtime containing functions related to file system, heap, loader and common tasks.
* 
* @date		20/03/2008
* @author	Pablo Bravo
*/
/******************************************************************************/
#include "RTL.h"
#include "XFS.h"
#include "HardDisk.h"

#include "Debug.h"

//==================================DATA======================================//
#pragma data_seg(".data")
//============================================================================//
/**
* @brief Heap Node definition for kernel Heap.
*/
union HEAP_NODE
{
	/**
	* @brief Heap Node definition when the node is free.
	*/
	struct
	{
		dword		magic;
		HEAP_NODE*	next;
		HEAP_NODE*	back;
	} freed;
	/**
	* @brief Heap Node definition when the node is allocated.
	*/
	struct
	{
		dword	blocks;
		byte	empty[60];
	} allocated;
};

/**
* @brief Heap definition for kernel Heap.
*/
struct HEAP
{
	HEAP_NODE*	ptr;
	dword		size;
	dword		number;
};

/**
* @brief XFS entry directory.
*/
PRIVATE XFS_ENTRY* rtl_xfs_directory = 0;

/**
* @brief Runtime kernel heap.
*/
PRIVATE HEAP rtl_heap = {0, 0, 0};

#define HEAP_MAGIC			'HEAP'
#define HEAP_SIZE_IN_BYTES	(1024*1024)
#define HEAP_SIZE_IN_PAGES	(HEAP_SIZE_IN_BYTES/PAGE_SIZE)
#define HEAP_NODES_NUMBER	(HEAP_SIZE_IN_BYTES/sizeof(HEAP_NODE))

//==================================CODE======================================//
#pragma code_seg(".code")
//============================================================================//
PRIVATE bool HEAP_Init();
PRIVATE bool FILE_Init(IN DISK_LOADER_DATA* _loader_data);

/**
* @brief Initialization of runtime kernel systems, heap and file system.
* @param _loader_data [in] Data of the disk for loading XFS.
* @return True if all goes ok, false otherwise.
*/
PUBLIC bool RTL_Init(IN DISK_LOADER_DATA* _loader_data)
{
	//Inicialize heap support.
	if(!HEAP_Init())
		return false;

	//Inicialize file system support.
	if(!FILE_Init(_loader_data))
		return false;
	
	//All OK
	return true;
}

/**
* @brief Copies _size bytes from _origin to _destiny.
* @param _destiny [in] Destiny address.
* @param _origin [in] Source addres.
* @param _size [in] Bytes to copy.
*/
PUBLIC NAKED void RTL_Copy(IN PHYSICAL _destiny, IN PHYSICAL _origin, IN dword _size)
{
	__asm
	{
		push ebp
		mov ebp, esp
		push ecx
		push esi
		push edi
		mov ecx, dword ptr [ebp + 16] //_size
		mov esi, dword ptr [ebp + 12] //_origin
		mov edi, dword ptr [ebp + 8]  //_destiny
		rep movsb
		pop edi
		pop esi
		pop ecx
		pop ebp
		ret 12
	}
}

/**
* @brief Translates a given number of bytes to the number of other element which have size greater than one.
* @param _bytes [in] Number of bytes.
* @param _unit_size [in] Size of the element.
* @return Number of elements of _unit_size given _bytes number.
*/
PUBLIC dword RTL_BytesToUnit(IN dword _bytes, IN dword _unit_size)
{
	return (_bytes/_unit_size) + ((_bytes%_unit_size)?1:0);
}

/**
* @brief Given a byte offset, translates to an offset of another element with different size.
* @param _offset [in] Byte offset.
* @param _unit_size [in] Size of the element.
* @return Offset in elements of _unit_size given _offset in bytes.
*/
PUBLIC dword RTL_ByteOffsetToUnitOffset(IN dword _offset, IN dword _unit_size)
{
	return _offset/_unit_size;
}

/**
* @brief Translates a given number _bytes into a number of sectors.
* @param _bytes [in] Number of bytes.
* @return Number of sectors.
*/
PUBLIC dword RTL_BytesToSectors(IN dword _bytes)
{
	return RTL_BytesToUnit(_bytes, SECTOR_SIZE);
}

/**
* @brief Translates a given number _bytes into a number of pages.
* @param _bytes [in] Number of bytes.
* @return Number of pages.
*/
PUBLIC dword RTL_BytesToPages(IN dword _bytes)
{
	return RTL_BytesToUnit(_bytes, PAGE_SIZE);
}

/**
* @brief Translates a given number _bytes into a number of heap nodes.
* @param _bytes [in] Number of bytes.
* @return Number of heap nodes.
*/
PUBLIC dword RTL_BytesToHeapNodes(IN dword _bytes)
{
	return RTL_BytesToUnit(_bytes, sizeof(HEAP_NODE));
}

/**
* @brief Translates a given offset in _bytes into a LBA offset.
* @param _offset [in] Offset in bytes.
* @return LBA offset.
*/
PUBLIC dword RTL_ByteOffsetToLBA(IN dword _offset)
{
	return RTL_ByteOffsetToUnitOffset(_offset, SECTOR_SIZE);
}

/**
* @brief Compares two strings.
* @param _s1 [in] First string.
* @param _s2 [in] Second string.
* @return True if strings are equal, false otherwise.
*/
PUBLIC bool STRING_Compare(IN string* _s1, IN string* _s2)
{
	if(_s1->size != _s2->size)
		return false;

	for(byte i = 0; i < _s1->size; i++)
	{
		if(_s1->text[i] != _s2->text[i])
			return false;
	}

	return true;
}

/**
* @brief Appends two strings.
* @param _s1 [in] First string.
* @param _s2 [in] Second string.
* @return True if appended.
*/
PUBLIC bool STRING_Append(IN OUT string* _s1, IN string* _s2)
{
	//Assumming enough space in s1
	for(byte i = 0; i < _s2->size; i++)
		_s1->text[_s1->size++] = _s2->text[i];

	return true;
}

/**
* @brief Copy one string into another.
* @param _s1 [in] First string.
* @param _s2 [in] Second string.
* @return True if string was copied.
*/
PUBLIC bool STRING_Copy(OUT string* _s1, IN string* _s2)
{
	//Assumming enough space in s1
	for(byte i = 0; i < _s2->size; i++)
		_s1->text[i] = _s2->text[i];

	_s1->size = _s2->size;
	return true;
}

/**
* @brief Loads XFS directories recursively.
* @param _lba [in] XFS entry in XFT disk address.
* @return The XFS_ENTRY* representing the file system.
*/
PRIVATE XFS_ENTRY* FILE_LoadXFS(IN LBA _lba)
{
	XFS_ENTRY* directory = (XFS_ENTRY*)MEM_AllocPages(1, KernelMode);
	if(directory)
	{
		//Read 8 sectors
		if(HD_ReadSectors(0, _lba, XFS_DIRECTORY_SECTORS, (VIRTUAL)directory) != XFS_DIRECTORY_SECTORS)
		{
			MEM_ReleasePages((PHYSICAL)directory, 1);
			return 0;
		}
		//Load recursively all directories
		//We skip PARENT
		//Check the entry is valid (used!=0)
		for(dword index = 1; directory[index].used; index++)
		{
			//Check the entry is a directory (is_directory==1)
			if(directory[index].is_directory)
			{
				XFS_ENTRY* son_directory = FILE_LoadXFS(RTL_ByteOffsetToLBA(directory[index].direction));
				if(son_directory)
				{
					//Redirect to memory
					directory[index].direction = (dword)son_directory;
				}
				else
				{
					//Could not read... unusable
					directory[index].used = 0;
				}
			}
		}
		return directory;
	}
	return 0;
}

/**
* @brief Initializes kernel file system support. Reads XFS directory tree in memory.
* @param _loader_data [in] Disk data loader.
* @return True if directory can be read, false otherwise.
*/
PRIVATE bool FILE_Init(IN DISK_LOADER_DATA* _loader_data)
{
	//Xky File Table
	XFS_ENTRY* xft = (XFS_ENTRY*)_loader_data->xft;
	
	//Skip LDR(0-entry) y del KRNL(1-entry)
	dword lba = RTL_ByteOffsetToLBA(xft[2].direction);

	rtl_xfs_directory = FILE_LoadXFS(lba);
	
	return rtl_xfs_directory != 0;
}

/**
* @brief Indicates if a given path has directories in it.
* @param _file_path [in] The path to test.
* @return True if path is anidated (has directories), false otherwise.
*/
PRIVATE bool FILE_PathHasDirectories(IN string* _file_path)
{
	for(byte i = 0; i < _file_path->size; i++)
	{
		if(_file_path->text[i] == '\\')
			return true;
	}
	return false;
}

/**
* @brief Gets a substring from an anidated path.
* As an exmaple, if _file_path contains DIR1\DIR2\FILE1, the function returns DIR1
* @param _file_path [in] The path to get a substring from.
* @return The substring, if there wasn't any problem, 0 otherwise.
*/
PRIVATE string* FILE_GetFirstDirectoryFromPath(IN string* _file_path)
{
	for(byte i = 0; i < _file_path->size; i++)
	{
		if(_file_path->text[i] == '\\')
		{
			string* sub_string = (string*)HEAP_Alloc(i);
			if(sub_string)
			{
				sub_string->size = i;
				for(byte j = 0; j < i; j++)
				{
					sub_string->text[j] = _file_path->text[j];
				}
				return sub_string;
			}
			return 0;
		}
	}
	return 0;
}

/**
* @brief Gets a substring from an anidated path.
* As an example, if _file_path contains DIR1\DIR2\FILE1, the function returns DIR2\FILE1
* @param _file_path [in] The path to get a substring from.
* @return The substring, if there wasn't any problem, 0 otherwise.
*/
PRIVATE string* FILE_SkipFirstDirectoryFromPath(IN string* _file_path)
{
	for(byte i = 0; i < _file_path->size; i++)
	{
		if(_file_path->text[i] == '\\')
		{
			string* sub_string = (string*)HEAP_Alloc(_file_path->size - i);
			if(sub_string)
			{
				i++;
				sub_string->size = _file_path->size - i;
				for(byte j = i; j < _file_path->size; j++)
				{
					sub_string->text[j-i] = _file_path->text[j];
				}
				return sub_string;
			}
			return 0;
		}
	}
	return 0;
}
/**
* @brief Search for a file recursively.
* @param _directory [in] Directory XFS entry.
* @param _file_path [in] Path of the file we are searching.
* @return The XFS_ENTRY where the file is located.
*/
PRIVATE XFS_ENTRY* FILE_SearchRecursive(IN XFS_ENTRY* _directory, IN string* _file_path)
{
	if(FILE_PathHasDirectories(_file_path))
	{
		string* directory_name = FILE_GetFirstDirectoryFromPath(_file_path);
		XFS_ENTRY* entry = 0;

		if(directory_name)
		{
			//We browse the directory
			//We skip PARENT
			//Check the entry is valid (used!=0)
			for(dword index = 1; _directory[index].used; index++)
			{
				//Check the entry is a directory (is_directory==1)
				if(_directory[index].is_directory)
				{
					string* entry_name = (string*)&_directory[index].name_size;

					if(STRING_Compare(entry_name, directory_name))
					{
						XFS_ENTRY* new_directory = (XFS_ENTRY*) _directory[index].direction;

						string* new_file_path = FILE_SkipFirstDirectoryFromPath(_file_path);

						entry = FILE_SearchRecursive(new_directory, new_file_path);

						HEAP_Free((PHYSICAL&)new_file_path);

						break;
					}
				}
			}
			HEAP_Free((PHYSICAL&)directory_name);
		}
		return entry;
	}
	else
	{
		//We browse the directory
		//We skip PARENT
		//Check the entry is valid (used!=0)
		
		for(dword index = 1; _directory[index].used; index++)
		{
			//Check the entry is not a directory (is_directory!=1)
			if(!_directory[index].is_directory)
			{
				string* entry_name = (string*)&_directory[index].name_size;

				if(STRING_Compare(entry_name, _file_path))
				{
					return &_directory[index];
				}
			}
		}
	}
	return 0;
}

/**
* @brief Search for a file.
* @param _file_path [in] Path of the file we are searching.
* @return The XFS_ENTRY where the file is located.
*/
PRIVATE XFS_ENTRY* FILE_Search(IN string* _file_path)
{
	return FILE_SearchRecursive(rtl_xfs_directory, _file_path);
}

/**
* @brief Test if a given file exists.
* @param _file_path [in] Path of the file we are testing.
* @return True if the file exists, false otherwise.
*/
PUBLIC bool FILE_Exists(IN string* _file_path)
{
	return FILE_Search(_file_path) != 0;
}

/**
* @brief Gets the size of a given file.
* @param _file_path [in] Path of the file we want to know its size.
* @return The file size or zero if the file does not exist.
*/
PUBLIC dword FILE_Size(IN string* _file_path)
{
	XFS_ENTRY* file = FILE_Search(_file_path);
	if(file)
	{
		return file->size;
	}
	return 0;
}

/**
* @brief Reads a file in a buffer.
* @param _file_path [in] Path of the file we want to read.
* @param _memory [out] Memory buffer big enough to contain the file in sectors.
* @return True if the file was read, false otherwise.
*/
PUBLIC bool FILE_Read(IN string* _file_path, OUT byte* _memory)
{
	XFS_ENTRY* file = FILE_Search(_file_path);
	if(file)
	{
		//Get lba address
		LBA lba = RTL_ByteOffsetToLBA(file->direction);
		//Get number of sectors to read
		dword sectors = RTL_BytesToSectors(file->size);

		return HD_ReadSectors(0, lba, sectors, (VIRTUAL)_memory) == sectors;
	}
	return false;
}

/**
* @brief Initializes the heap.
* @return True if initilization was successful, false otherwise.
*/
PRIVATE bool HEAP_Init()
{
	//Ask for memory for heap
	rtl_heap.ptr = (HEAP_NODE*)MEM_AllocPages(HEAP_SIZE_IN_PAGES, KernelMode);
	if(rtl_heap.ptr)
	{
		//Memory allocated
		rtl_heap.size = HEAP_SIZE_IN_PAGES;
		rtl_heap.number = HEAP_NODES_NUMBER;
		
		//Partitionate Heap
		for(dword i = 1; i < HEAP_NODES_NUMBER - 1; i++)
		{
			rtl_heap.ptr[i].freed.magic = HEAP_MAGIC;
			rtl_heap.ptr[i].freed.next = &rtl_heap.ptr[i + 1];
			rtl_heap.ptr[i].freed.back = &rtl_heap.ptr[i - 1];
		}
		
		//Fix first and last node
		rtl_heap.ptr[0].freed.magic = HEAP_MAGIC;
		rtl_heap.ptr[0].freed.next = &rtl_heap.ptr[1];
		rtl_heap.ptr[0].freed.back = 0;

		rtl_heap.ptr[HEAP_NODES_NUMBER-1].freed.magic = HEAP_MAGIC;
		rtl_heap.ptr[HEAP_NODES_NUMBER-1].freed.next = 0;
		rtl_heap.ptr[HEAP_NODES_NUMBER-1].freed.back = &rtl_heap.ptr[HEAP_NODES_NUMBER-2];

		//Ok
		return true;
	}
	//Not enough memory...
	return false;
}

/**
* @brief Initializes the heap.
* @param _node [in out] First node in a heap allocation.
* @param _number of nodes [in] Number of nodes to be allocated.
* @return Address of usable heap memory.
*/
PRIVATE PHYSICAL HEAP_AllocNodes(IN OUT HEAP_NODE* _node, IN dword _number_of_nodes)
{
	HEAP_NODE* previous = _node->freed.back;
	HEAP_NODE* last = _node + _number_of_nodes;

	//Relink
	if(previous)
		previous->freed.next = last;
	if(last)
		last->freed.back = previous;

	//Reserve
	for(dword i = 0; i < _number_of_nodes; i++)
	{
		_node[i].freed.magic = 0;
	}
	
	_node->allocated.blocks = _number_of_nodes;
	return (PHYSICAL)&_node->allocated.empty;
}

/**
* @brief Finds empty space in the heap for a number of nodes.
* @param _number_of_nodes [in] Number of nodes to be allocated.
* @return First node that satisfies that is empty, and _number_of_nodes - 1 next nodes are also free.
*/
PRIVATE HEAP_NODE* HEAP_FindEmptySpace(IN dword _number_of_nodes)
{
	for(dword i = 0; i < HEAP_NODES_NUMBER - _number_of_nodes; i++)
	{
		dword j;
		for(j = i; j < i + _number_of_nodes; j++)
		{
			if(rtl_heap.ptr[j].freed.magic != HEAP_MAGIC)
				break;
		}
		if(j == i + _number_of_nodes)
			return &rtl_heap.ptr[i];
	}
	return 0;
}

/**
* @brief Allocates empty space in the heap.
* @param _size [in] Number of bytes to be allocated.
* @return Address to be used, or zero if there was an error.
*/
PUBLIC PHYSICAL HEAP_Alloc(IN dword _size)
{
	dword number_of_nodes = RTL_BytesToHeapNodes(_size);
	if(number_of_nodes)
	{
		HEAP_NODE* nodes = HEAP_FindEmptySpace(number_of_nodes);
		if(nodes)
		{
			return HEAP_AllocNodes(nodes, number_of_nodes);
		}
	}
	return 0;
}

/**
* @brief Frees heap memory.
* @param _address [in] Address given in a previous call to HEAP_Alloc.
*/
PUBLIC void HEAP_Free(IN PHYSICAL& _address)
{
	_address = 0;
	return;

	/*
	if(!_address)
		return;

	HEAP_NODE* node_start = (HEAP_NODE*)(_address - 4);
	dword number = node_start->allocated.blocks;

	for(dword i = 1; i < number - 1; i++)
	{
		node_start[i].freed.magic = HEAP_MAGIC;
		node_start[i].freed.next = &node_start[i + 1];
		node_start[i].freed.back = &node_start[i - 1];
	}

	//First element must find first free back form him
	if(rtl_heap.ptr == node_start)
	{
		node_start[0].freed.back = 0;
	}
	else
	{
		HEAP_NODE* previous = node_start - 1;
		while(previous != rtl_heap.ptr)
		{
			if(previous->freed.magic == HEAP_MAGIC)
			{
				node_start[0].freed.back = previous;
				break;
			}
			else
				previous--;
		}

		if(rtl_heap.ptr == previous)
		{
			if(previous->freed.magic == HEAP_MAGIC)
			{
				node_start[0].freed.back = previous;
			}
			else
			{
				node_start[0].freed.back = 0;
			}
		}
	}


	//Last element must find first free next form him
	if(&rtl_heap.ptr[HEAP_NODES_NUMBER-1] == node_start + number)
	{
		node_start[number].freed.next = 0;
	}
	else
	{
		HEAP_NODE* next = node_start + number;
		while(next != &rtl_heap.ptr[HEAP_NODES_NUMBER-1])
		{
			if(next->freed.magic == HEAP_MAGIC)
			{
				node_start[number].freed.next = next;
				break;
			}
			else
				next++;
		}

		if(&rtl_heap.ptr[HEAP_NODES_NUMBER-1] == next)
		{
			if(next->freed.magic == HEAP_MAGIC)
			{
				node_start[number].freed.back = next;
			}
			else
			{
				node_start[number].freed.next = 0;
			}
		}
	}

	_address = 0;
	*/
}

/**
* @brief Initializes a list.
* @param _head [out] The head of the list.
*/
PUBLIC void LIST_Init(OUT LIST_ENTRY* _head)
{
	_head->back = _head;
	_head->next = _head;
}

/**
* @brief Test if a given list is empty.
* @param _head [in] The head of the list.
* @return True if the list is empty, false otherwise.
*/
PUBLIC bool LIST_IsEmpty(IN LIST_ENTRY* _head)
{
	return (_head->back == _head) &&  (_head->next == _head);
}

/**
* @brief Inserts an element at the head of the list.
* @param _head [in out] The head of the list.
* @param _element [in out] The element to insert.
*/
PUBLIC void LIST_InsertHead(IN OUT LIST_ENTRY* _head, IN OUT LIST_ENTRY* _element)
{
	LIST_ENTRY* next = _head->next;

	_element->next = next;
	_element->back = _head;

	_head->next = _element;
	next->back = _element;
}

/**
* @brief Inserts an element at the tail of the list.
* @param _head [in out] The head of the list.
* @param _element [in out] The element to insert.
*/
PUBLIC void LIST_InsertTail(IN OUT LIST_ENTRY* _head, IN OUT LIST_ENTRY* _element)
{
	LIST_ENTRY* back = _head->back;

	_element->next = _head;
	_element->back = back;

	_head->back = _element;
	back->next = _element;
}

/**
* @brief Removes an element from a list.
* @param _element [in out] The element to remove.
*/
PUBLIC void LIST_Remove(IN OUT LIST_ENTRY* _element)
{
	LIST_ENTRY* next = _element->next;
	LIST_ENTRY* back = _element->back;

	next->back = back;
	back->next = next;

	_element->next = _element;
	_element->back = _element;
}

/**
* @brief Iterates through a list.
* @param _head [in] The head of the list.
* @return An iterator for next calls and to access elements.
*/
PUBLIC LIST_ITERATOR LIST_First(IN LIST_ENTRY* _head)
{
	if(LIST_IsEmpty(_head))
		return 0;
	return _head->next;
}

/**
* @brief Iterates through a list.
* @param _head [in] The head of the list.
* @param _iterator [in] The iterator obtained in a previous call.
* @return An iterator for the next element, 0 when no more elements.
*/
PUBLIC LIST_ITERATOR LIST_Next(IN LIST_ENTRY* _head, IN LIST_ITERATOR _iterator)
{
	if(!_iterator || !_head)
		return 0;

	if(_iterator->next == _head)
		return 0;

	return _iterator->next;
}


/**
* @brief Loads an image.
* @param _module_name [in] The module name.
* @param _mode [in] Indicates kernel or user memory mode.
* @return The physical address the module gets loaded, or zero if there was an error.
*/
PUBLIC PHYSICAL LDR_LoadImage(IN string* _module_name, IN ExecutionType _mode)
{
	dword size = FILE_Size(_module_name);
	if(size)
	{
		dword number_of_pages = RTL_BytesToPages(size);
		PHYSICAL memory = MEM_AllocPages(number_of_pages, _mode);
		if(memory)
		{
			if(FILE_Read(_module_name, (byte*)memory))
			{
				//Check
				IMG_MODULE_HEADER* module = (IMG_MODULE_HEADER*) memory;
				if(module->signature != IMAGE_SIGNATURE)
				{
					DEBUG_DATA("LDR_LoadImage failed signature = ", module->signature, 0x00FF0000)
					DEBUG_DATA("LDR_LoadImage module address   = ", memory, 0x00FF0000)
					MEM_ReleasePages(memory, number_of_pages);
					return 0;
				}
				if(module->file_header.mode > (dword)_mode)
				{
					DEBUG("LDR_LoadImage failed mode")
					MEM_ReleasePages(memory, number_of_pages);
					return 0;
				}

				//Ok
				return memory;
			}
			else
			{
				MEM_ReleasePages(memory, number_of_pages);
			}
		}
	}
	return 0;
}

/**
* @brief Get the relocs of a given module image.
* @param _module [in] The module image mapped in memory.
* @return The address of the relocs within the module.
*/
PRIVATE IMG_RELOCATION* LDR_GetRelocs(IN IMG_MODULE_HEADER* _module)
{
	if(!_module->relocs_section.size)
		return 0;
	
	return (IMG_RELOCATION*)(((byte*)_module) + _module->relocs_section.offset);
}

/**
* @brief Reubicates an image to a new base.
* @param _module [in] The module image to be rebased.
* @param _base [in] The new base.
*/
PUBLIC void LDR_ReubicateImage(IN IMG_MODULE_HEADER* _module, IN VIRTUAL _base)
{
	//Get relocs
	IMG_RELOCATION* relocs = LDR_GetRelocs(_module);
	if(relocs)
	{
		for(dword i = 0; i < relocs->number_of_relocations; i++)
		{
			dword offset = relocs->relocations[i];

			*(dword*)(((byte*)_module) + offset) += (dword)_base;
		}
	}
}

/**
* @brief Get the imports of a given module image.
* @param _module [in] The module image mapped in memory we want the imports.
* @param _start [out] The offset start of the imports.
* @param _end [out] The offset end of the imports.
* @return True if there are imports, false otherwise.
*/
PRIVATE bool LDR_GetImports(IN IMG_MODULE_HEADER* _module, OUT dword& _start, OUT dword& _end)
{
	if(!_module->imports_section.size)
		return false;

	_start = _module->imports_section.offset;
	_end = _start + _module->imports_section.size;
	return true;
}

/**
* @brief Get the exports of a given module image.
* @param _module [in] The module image mapped in memory we want the exports.
* @param _start [out] The offset start of the exports.
* @param _end [out] The offset end of the exports.
* @return True if there are exports, false otherwise.
*/
PRIVATE bool LDR_GetExports(IN IMG_MODULE_HEADER* _module, OUT dword& _start, OUT dword& _end)
{
	if(!_module->exports_section.size)
		return false;

	_start = _module->exports_section.offset;
	_end = _start + _module->exports_section.size;
	return true;
}

/**
* @brief Resolves an import within a given export module.
* @param _import [in out] The import we want to fill.
* @param _module [in] The export module.
* @return True if import was resolved, false otherwise.
*/
PRIVATE bool LDR_ResolveImportWithinModule(IN OUT IMG_IMPORT* _import, IN IMG_MODULE_HEADER* _module)
{
	dword exports_start, exports_end;

	if(LDR_GetExports(_module, exports_start, exports_end))
	{
		dword exports_number = (exports_end - exports_start)/sizeof(IMG_EXPORT);

		IMG_EXPORT* exports = (IMG_EXPORT*)((byte*)_module + exports_start);

		for(dword i = 0; i < exports_number; i++)
		{
			//Check if names are equal
			if(STRING_Compare((string*)_import, (string*)(exports + i)))
			{
				//Fill import with export address
				_import->function = exports[i].function;
				return true;
			}
		}
	}
	return false;
}

/**
* @brief Resolves all import within a given module with an export module.
* @param _module [in out] The module we want to resolve its imports.
* @param _export_module [in] The export module.
* @return True if all imports were resolved, false otherwise.
*/
PUBLIC bool LDR_ResolveImports(IN IMG_MODULE_HEADER* _module, IN IMG_MODULE_HEADER* _export_module)
{
	dword imports_start, imports_end;

	if(LDR_GetImports(_module, imports_start, imports_end))
	{
		dword imports_number = (imports_end - imports_start)/sizeof(IMG_IMPORT);

		IMG_IMPORT* imports = (IMG_IMPORT*)((byte*)_module + imports_start);

		for(dword i = 0; i < imports_number; i++)
		{
			if(!LDR_ResolveImportWithinModule(imports + i, _export_module))
			{
				DEBUG_Message((string*)(imports + i), 0x0000FF00);
				DEBUG_DATA("LDR_ResolveImport = ", i, 0x0000FF00)
				DEBUG_DATA("ExportModule = ",(dword) _export_module, 0x0000FF00)
				return false;
			}
		}
	}
	return true;
}

/**
* @brief Obtains the address of a function exported by a module.
* @param _module [in] The module we want to resolve its export.
* @param _function_name [in] The function name exported.
* @return The function address, if resolved. Zero otherwise.
*/
PUBLIC VIRTUAL LDR_GetProcedureAddress(IN IMG_MODULE_HEADER* _module, IN string* _function_name)
{
	dword exports_start, exports_end;

	if(LDR_GetExports(_module, exports_start, exports_end))
	{
		dword exports_number = (exports_end - exports_start)/sizeof(IMG_EXPORT);

		IMG_EXPORT* exports = (IMG_EXPORT*)((byte*)_module + exports_start);

		for(dword i = 0; i < exports_number; i++)
		{
			//Check if names are equal
			if(STRING_Compare(_function_name, (string*)&exports[i]))
			{
				//Fill import with export address
				return exports[i].function;
			}
		}
	}
	return 0;
}

/**
* @brief Loads a file.
* @param _name [in] The file name.
* @param _mode [in] Indicates kernel or user memory mode.
* @return The address the file gets mapped, or zero if there was an error.
*/
PUBLIC PHYSICAL LDR_LoadFile(IN string* _name, IN ExecutionType _mode)
{
	dword size = FILE_Size(_name);
	if(size)
	{
		dword number_of_pages = RTL_BytesToPages(size);
		PHYSICAL memory = MEM_AllocPages(number_of_pages, _mode);
		if(memory)
		{
			if(FILE_Read(_name, (byte*)memory))
			{
				//Ok
				return memory;
			}
			else
			{
				MEM_ReleasePages(memory, number_of_pages);
			}
		}
	}
	return 0;
}
