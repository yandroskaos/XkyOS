/******************************************************************************/
/**
* @file		IO.cpp
* @brief	XkyOS Hardware IO Library
* Implementation of IO operations and types
* 
* @date		20/03/2008
* @author	Pablo Bravo
*/
/******************************************************************************/
#include "Types.h"
#include "IO.h"

//==================================CODE======================================//
#pragma code_seg(".code")
//============================================================================//

/**
* @brief Reads one byte from an io port.
* @param _port [in] Port to read from.
* @return Returns the data byte.
*/
PUBLIC NAKED byte IO_InPortByte(IN word _port)
{
	__asm
	{
		push edx
		xor eax, eax
		mov edx, dword ptr [esp + 8]
		in al, dx
		pop edx
		ret 4
	}
}

/**
* @brief Reads one word (2 bytes) from an io port.
* @param _port [in] Port to read from.
* @return Returns the data word.
*/
PUBLIC NAKED word IO_InPortWord(IN word _port)
{
	__asm
	{
		push edx
		xor eax, eax
		mov edx, dword ptr [esp + 8]
		in ax, dx
		pop edx
		ret 4
	}
}

/**
* @brief Reads one dword (4 bytes) from an io port.
* @param _port [in] Port to read from.
* @return Returns the data dword.
*/
PUBLIC NAKED dword IO_InPortDword(IN word _port)
{
	__asm
	{
		push edx
		xor eax, eax
		mov edx, dword ptr [esp + 8]
		in eax, dx
		pop edx
		ret 4
	}
}

/**
* @brief Writes one byte to an io port.
* @param _port [in] Port to write to.
* @param _data [in] Data to be written.
*/
PUBLIC NAKED void IO_OutPortByte(IN word _port, IN byte _data)
{
	__asm
	{
		push edx
		push eax
		mov edx, dword ptr [esp + 12]
		mov eax, dword ptr [esp + 16]
		out dx, al
		pop eax
		pop edx
		ret 8
	}
}

/**
* @brief Writes one word (2 bytes) to an io port.
* @param _port [in] Port to write to.
* @param _data [in] Data to be written.
*/
PUBLIC NAKED void IO_OutPortWord(IN word _port, IN word _data)
{
	__asm
	{
		push edx
		push eax
		mov edx, dword ptr [esp + 12]
		mov eax, dword ptr [esp + 16]
		out dx, ax
		pop eax
		pop edx
		ret 8
	}
}

/**
* @brief Writes one dword (4 bytes) to an io port.
* @param _port [in] Port to write to.
* @param _data [in] Data to be written.
*/
PUBLIC NAKED void IO_OutPortDword(IN word _port, IN dword _data)
{
	__asm
	{
		push edx
		push eax
		mov edx, dword ptr [esp + 12]
		mov eax, dword ptr [esp + 16]
		out dx, eax
		pop eax
		pop edx
		ret 8
	}
}
