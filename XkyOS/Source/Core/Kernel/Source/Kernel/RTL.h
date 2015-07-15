/******************************************************************************/
/**
* @file		RTL.h
* @brief	XkyOS Kernel Runtime Library
* Runtime containing functions related to file system, heap, loader and common tasks.
* 
* @date		20/03/2008
* @author	Pablo Bravo
*/
/******************************************************************************/
#ifndef __RTL_INCLUDES__
#define __RTL_INCLUDES__

	#include "Types.h"
	#include "Image.h"
	#include "Loader.h"
	#include "Memory.h"

	bool	RTL_Init(IN DISK_LOADER_DATA* _loader_data);

	//Conversion
	dword	RTL_BytesToUnit				(IN dword _bytes, IN dword _unit_size);
	dword	RTL_ByteOffsetToUnitOffset	(IN dword _offset, IN dword _unit_size);
	dword	RTL_BytesToSectors			(IN dword _bytes);
	dword	RTL_BytesToPages			(IN dword _bytes);
	dword	RTL_BytesToHeapNodes		(IN dword _bytes);
	dword	RTL_ByteOffsetToLBA			(IN dword _offset);

	void RTL_Copy(OUT PHYSICAL _destiny, IN PHYSICAL _origin, IN dword _size);

	//Strings
	bool STRING_Compare	(IN string* _s1, IN string* _s2);
	bool STRING_Append	(IN OUT string* _s1, IN string* _s2);
	bool STRING_Copy	(OUT string* _s1, IN string* _s2);

	//Files
	bool	FILE_Exists	(IN string* _file_path);
	dword	FILE_Size	(IN string* _file_path);
	bool	FILE_Read	(IN string* _file_path, OUT byte* _memory);

	//Heap
	PHYSICAL	HEAP_Alloc	(IN dword _size);
	void		HEAP_Free	(IN PHYSICAL& _address);

	//Lists
	struct LIST_ENTRY
	{
		LIST_ENTRY* next;
		LIST_ENTRY* back;
	};
	typedef LIST_ENTRY* LIST_ITERATOR;

	void	LIST_Init		(   OUT LIST_ENTRY* _head);
	bool	LIST_IsEmpty	(IN     LIST_ENTRY* _head);
	void	LIST_InsertHead	(IN OUT LIST_ENTRY* _head, IN OUT LIST_ENTRY* _element);
	void	LIST_InsertTail	(IN OUT LIST_ENTRY* _head, IN OUT LIST_ENTRY* _element);
	void	LIST_Remove		(IN OUT LIST_ENTRY* _element);

	LIST_ITERATOR	LIST_First	(IN LIST_ENTRY* _head);
	LIST_ITERATOR	LIST_Next	(IN LIST_ENTRY* _head, IN LIST_ITERATOR _iterator);

	//Loader
	PHYSICAL	LDR_LoadImage			(IN string* _module_name, IN ExecutionType _mode);
	void		LDR_ReubicateImage		(IN IMG_MODULE_HEADER* _module, IN VIRTUAL _base);
	bool		LDR_ResolveImports		(IN IMG_MODULE_HEADER* _module, IN IMG_MODULE_HEADER* _api);
	VIRTUAL		LDR_GetProcedureAddress	(IN IMG_MODULE_HEADER* _module, IN string* _function_name);
	PHYSICAL	LDR_LoadFile			(IN string* _name, IN ExecutionType _mode);

#endif
