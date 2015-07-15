/******************************************************************************/
/**
* @file		RTL.h
* @brief	Runtime Library User
*
* @date		15/04/2008
* @author	Pablo Bravo
*/
/******************************************************************************/
#ifndef __RTL_H__
#define __RTL_H__

//Init
typedef bool (*fRTL_Init)();

//Byte Copy
typedef void	(*fRTL_Copy)(IN VIRTUAL _destiny, IN VIRTUAL _origin, IN dword _size);

//Byte Translation
typedef dword	(*fRTL_BytesToUnit)				(IN dword _bytes, IN dword _unit_size);
typedef dword	(*fRTL_ByteOffsetToUnitOffset)	(IN dword _offset, IN dword _unit_size);
typedef dword	(*fRTL_BytesToSectors)			(IN dword _bytes);
typedef dword	(*fRTL_BytesToPages)			(IN dword _bytes);
typedef dword	(*fRTL_ByteOffsetToLBA)			(IN dword _offset);

//String operations
typedef bool (*fSTRING_Compare)	(IN string* _s1, IN string* _s2);
typedef bool (*fSTRING_Append)	(IN OUT string* _s1, IN string* _s2);
typedef bool (*fSTRING_Copy)	(OUT string* _s1, IN string* _s2);
typedef void (*fSTRING_ToString)(IN string* _buffer, IN dword _data, IN byte _size);

//Heap operations
typedef VIRTUAL	(*fHEAP_Alloc)	(IN dword _size);
typedef void	(*fHEAP_Free)	(IN VIRTUAL& _address);

//List operations
struct LIST_ENTRY
{
	LIST_ENTRY* next;
	LIST_ENTRY* back;
};
typedef LIST_ENTRY* LIST_ITERATOR;

typedef void			(*fLIST_Init)		(OUT LIST_ENTRY* _head);
typedef bool			(*fLIST_IsEmpty)	(IN LIST_ENTRY* _head);
typedef void			(*fLIST_InsertHead)	(IN OUT LIST_ENTRY* _head, IN OUT LIST_ENTRY* _element);
typedef void			(*fLIST_InsertTail)	(IN OUT LIST_ENTRY* _head, IN OUT LIST_ENTRY* _element);
typedef void			(*fLIST_Remove)		(IN OUT LIST_ENTRY* _element);
typedef LIST_ITERATOR	(*fLIST_First)		(IN LIST_ENTRY* _head);
typedef LIST_ITERATOR	(*fLIST_Next)		(IN LIST_ENTRY* _head, IN LIST_ITERATOR _iterator);

//Console operations
typedef void	(*fCONSOLE_Init)		(IN WINDOW _window, IN ARGB _color, IN string* _title, IN string* _prompt);
typedef void	(*fCONSOLE_SetCursor)	(IN dword _new_x, IN dword _new_y);
typedef dword	(*fCONSOLE_GetX)		();
typedef dword	(*fCONSOLE_GetY)		();
typedef void	(*fCONSOLE_Clear)		(IN ARGB _color);
typedef void	(*fCONSOLE_Write)		(IN string* _text, IN ARGB _color);
typedef void	(*fCONSOLE_NewLine)		();
typedef void	(*fCONSOLE_WriteLn)		(IN string* _text, IN ARGB _color);
typedef void	(*fCONSOLE_WriteNumber)	(IN dword _data, IN byte _size, IN ARGB _color);

#endif //__RTL_H__
