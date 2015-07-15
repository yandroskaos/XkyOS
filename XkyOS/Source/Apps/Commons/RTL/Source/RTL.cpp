/******************************************************************************/
/**
* @file		RTL.cpp
* @brief	Runtime Library User Mode
*
* @date		16/04/2008
* @author	Pablo Bravo García
*/
/******************************************************************************/
#include "Types.h"
#include "Executable.h"

//=================================IMPORTS====================================//
#pragma data_seg(".imports")
//============================================================================//
#include "API.h"

//==================================DATA======================================//
#pragma data_seg(".data")
//============================================================================//
#include "RTL.h"
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
* @brief Runtime heap.
*/
HEAP rtl_heap = {0, 0, 0};

#define HEAP_MAGIC			'HEAP'
#define HEAP_SIZE_IN_BYTES	(1024*1024)
#define HEAP_SIZE_IN_PAGES	(HEAP_SIZE_IN_BYTES/PAGE_SIZE)
#define HEAP_NODES_NUMBER	(HEAP_SIZE_IN_BYTES/sizeof(HEAP_NODE))

//CONSOLE constants
#define CHARACTER_HEIGHT	8
#define CHARACTER_WIDTH		8
#define HEIGHT_MARGIN		8
#define WIDTH_MARGIN		8

WINDOW window;
dword height;
dword width;
dword x;
dword y;

//==================================CODE======================================//
#pragma code_seg(".code")
//============================================================================//
PRIVATE bool HEAP_Init();

/**
* @brief Initialization of runtime systems.
* @return True if all goes ok, false otherwise.
*/
PUBLIC bool RTL_Init()
{
	//Inicialize heap support.
	if(!HEAP_Init())
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
PUBLIC NAKED void RTL_Copy(IN VIRTUAL _destiny, IN VIRTUAL _origin, IN dword _size)
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
PRIVATE dword RTL_BytesToHeapNodes(IN dword _bytes)
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
* @brief Translates a given data into a string.
* @param _buffer [in] String buffer.
* @param _data [in] Data to show.
* @param _size [in] Width of the data.
*/
PUBLIC void STRING_ToString(IN string* _buffer, IN dword _data, IN byte _size)
{
	if(_size > 8)
		_size = 8;

#define TO_CHAR(X) (((X)>9)?((X)+0x37):((X)+0x30))

	byte b1 = (byte)((_data & 0xFF000000) >> 24);
	byte b2 = (byte)((_data & 0x00FF0000) >> 16);
	byte b3 = (byte)((_data & 0x0000FF00) >> 8);
	byte b4 = (byte)(_data & 0x000000FF);

	byte nibble0 = ((b1&0xF0)>>4);
	byte nibble1 = (b1&0x0F);
	byte nibble2 = ((b2&0xF0)>>4);
	byte nibble3 = (b2&0x0F);
	byte nibble4 = ((b3&0xF0)>>4);
	byte nibble5 = (b3&0x0F);
	byte nibble6 = ((b4&0xF0)>>4);
	byte nibble7 = (b4&0x0F);

	_buffer->size = _size;
	if(_size == 1)
	{
		_buffer->text[0] = TO_CHAR(nibble7);
	}
	if(_size == 2)
	{
		_buffer->text[0] = TO_CHAR(nibble6);
		_buffer->text[1] = TO_CHAR(nibble7);
	}
	if(_size == 3)
	{
		_buffer->text[0] = TO_CHAR(nibble5);
		_buffer->text[1] = TO_CHAR(nibble6);
		_buffer->text[2] = TO_CHAR(nibble7);
	}
	if(_size == 4)
	{
		_buffer->text[0] = TO_CHAR(nibble4);
		_buffer->text[1] = TO_CHAR(nibble5);
		_buffer->text[2] = TO_CHAR(nibble6);
		_buffer->text[3] = TO_CHAR(nibble7);
	}
	if(_size == 5)
	{
		_buffer->text[0] = TO_CHAR(nibble3);
		_buffer->text[1] = TO_CHAR(nibble4);
		_buffer->text[2] = TO_CHAR(nibble5);
		_buffer->text[3] = TO_CHAR(nibble6);
		_buffer->text[4] = TO_CHAR(nibble7);
	}
	if(_size == 6)
	{
		_buffer->text[0] = TO_CHAR(nibble2);
		_buffer->text[1] = TO_CHAR(nibble3);
		_buffer->text[2] = TO_CHAR(nibble4);
		_buffer->text[3] = TO_CHAR(nibble5);
		_buffer->text[4] = TO_CHAR(nibble6);
		_buffer->text[5] = TO_CHAR(nibble7);
	}
	if(_size == 7)
	{
		_buffer->text[0] = TO_CHAR(nibble1);
		_buffer->text[1] = TO_CHAR(nibble2);
		_buffer->text[2] = TO_CHAR(nibble3);
		_buffer->text[3] = TO_CHAR(nibble4);
		_buffer->text[4] = TO_CHAR(nibble5);
		_buffer->text[5] = TO_CHAR(nibble6);
		_buffer->text[6] = TO_CHAR(nibble7);
	}
	if(_size == 8)
	{
		_buffer->text[0] = TO_CHAR(nibble0);
		_buffer->text[1] = TO_CHAR(nibble1);
		_buffer->text[2] = TO_CHAR(nibble2);
		_buffer->text[3] = TO_CHAR(nibble3);
		_buffer->text[4] = TO_CHAR(nibble4);
		_buffer->text[5] = TO_CHAR(nibble5);
		_buffer->text[6] = TO_CHAR(nibble6);
		_buffer->text[7] = TO_CHAR(nibble7);
	}
}

/**
* @brief Initializes the heap.
* @return True if initilization was successful, false otherwise.
*/
PRIVATE bool HEAP_Init()
{
	//Ask for memory for heap
	rtl_heap.ptr = (HEAP_NODE*)XKY_PAGE_Alloc(XKY_ADDRESS_SPACE_GetCurrent(), 0x30000000, HEAP_SIZE_IN_PAGES);
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
PRIVATE VIRTUAL HEAP_AllocNodes(IN OUT HEAP_NODE* _node, IN dword _number_of_nodes)
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
	return (VIRTUAL)&_node->allocated.empty;
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
PUBLIC VIRTUAL HEAP_Alloc(IN dword _size)
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
PUBLIC void HEAP_Free(IN VIRTUAL& _address)
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
* @brief Test if a given list is empty
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
* @brief Clears the console with a color.
* @param _color [in] The color.
*/
PUBLIC void CONSOLE_Clear(IN ARGB _color)
{
	//Rellenar el rectangulo
	for(dword x = WIDTH_MARGIN; x <= (width - WIDTH_MARGIN); x++)
	{
		for(dword y = HEIGHT_MARGIN; y <= height - HEIGHT_MARGIN; y++)
		{
			XKY_WINDOW_SetPixel(window, x, y, _color);
		}
	}
}

/**
* @brief Writes a string in the console.
* @param _text [in] The string to write.
* @param _color [in] The color of the string.
*/
PUBLIC void CONSOLE_Write(IN string* _text, IN ARGB _color)
{
	if(_text->size && (x + _text->size * CHARACTER_WIDTH < width - WIDTH_MARGIN))
	{
		XKY_WINDOW_PrintText(window, x, y, _color, _text);
		x += _text->size * CHARACTER_WIDTH;
	}
}

/**
* @brief Writes an hexadecimal number to the console.
* @param _data [in] The data to write.
* @param _size [in] The width of the data.
* @param _color [in] The color of the string.
*/
PUBLIC void CONSOLE_WriteNumber(IN dword _data, IN byte _size, IN ARGB _color)
{
	if(_size)
	{
		string buffer = STRING("00000000");
		STRING_ToString(&buffer, _data, _size);
		CONSOLE_Write(&buffer, _color);
	}
}

/**
* @brief Writes a new line in the console.
*/
PUBLIC void CONSOLE_NewLine()
{
	x = WIDTH_MARGIN;

	y -= CHARACTER_HEIGHT;
	if(y <= HEIGHT_MARGIN)
		y = height - HEIGHT_MARGIN;
}

/**
* @brief Writes a string in the console and then a new line.
* @param _text [in] The string to write.
* @param _color [in] The color of the string.
*/
PUBLIC void CONSOLE_WriteLn(IN string* _text, IN ARGB _color)
{
	CONSOLE_Write(_text, _color);
	CONSOLE_NewLine();
}

/**
* @brief Specifies the location of the cursor in the console.
* @param _new_x [in] The x coordinate.
* @param _new_y [in] The y coordinate.
*/
PUBLIC void CONSOLE_SetCursor(IN dword _new_x, IN dword _new_y)
{
	if((x <= width - WIDTH_MARGIN) && (y <= height - HEIGHT_MARGIN) && (x >= WIDTH_MARGIN) && (y >= HEIGHT_MARGIN))
	{
		x = _new_x;
		y = _new_y;
	}
}

/**
* @brief Obtains the location of coordinate x in the console.
* @return The x coordinate of the cursor.
*/
PUBLIC dword CONSOLE_GetX()
{
	return x;
}

/**
* @brief Obtains the location of coordinate y in the console.
* @return The y coordinate of the cursor.
*/
PUBLIC dword CONSOLE_GetY()
{
	return y;
}

/**
* @brief Initializes the console over a window.
* @param _window [in] The window we wiil put a console on.
* @param _color [in] The color of the console.
* @param _title [in] The title in the window.
* @param _prompt [in] The prompt in the window.
*/
PUBLIC void CONSOLE_Init(IN WINDOW _window, IN ARGB _color, IN string* _title, IN string* _prompt)
{
	window = _window;

	height = XKY_WINDOW_GetHeight(window);
	width = XKY_WINDOW_GetWidth(window);

	x = WIDTH_MARGIN;
	y = height - HEIGHT_MARGIN;

	//Colocamos la consola
	CONSOLE_Clear(_color);

	//Colocamos el nombre
	XKY_WINDOW_PrintText(window, 0, height, SRGB(255, 0, 0), _title);

	//Colocamos el prompt
	CONSOLE_Write(_prompt, SRGB(0, 255, 0));
}

//=================================EXPORTS====================================//
#pragma data_seg(".exports")
//============================================================================//
EXPORT(RTL_Init);

EXPORT(RTL_Copy);

EXPORT(RTL_BytesToUnit);
EXPORT(RTL_ByteOffsetToUnitOffset);
EXPORT(RTL_BytesToSectors);
EXPORT(RTL_BytesToPages);
EXPORT(RTL_ByteOffsetToLBA);

EXPORT(STRING_Compare);
EXPORT(STRING_Append);
EXPORT(STRING_Copy);
EXPORT(STRING_ToString);

EXPORT(HEAP_Alloc);
EXPORT(HEAP_Free);

EXPORT(LIST_Init);
EXPORT(LIST_IsEmpty);
EXPORT(LIST_InsertHead);
EXPORT(LIST_InsertTail);
EXPORT(LIST_Remove);
EXPORT(LIST_First);
EXPORT(LIST_Next);

EXPORT(CONSOLE_Init);
EXPORT(CONSOLE_SetCursor);
EXPORT(CONSOLE_GetX);
EXPORT(CONSOLE_GetY);
EXPORT(CONSOLE_Clear);
EXPORT(CONSOLE_Write);
EXPORT(CONSOLE_NewLine);
EXPORT(CONSOLE_WriteLn);
EXPORT(CONSOLE_WriteNumber);

//=================================MODULE=====================================//
#pragma data_seg(".module")
//============================================================================//
	MODULE(IMAGE_MODE_USER, IMAGE_KIND_MODULE, IMAGE_VERSION(1,0,0,0), 0, 0, 0);
