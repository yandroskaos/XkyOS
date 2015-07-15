/******************************************************************************/
/**
* @file		Debug.h
* @brief	Debug Subsystem
* 
* @date		20/03/2008
* @author	Pablo Bravo
*/
/******************************************************************************/
#ifndef __DEBUG_H__
#define __DEBUG_H__

#include "Types.h"
#include "..\Kernel\Windows.h"

//#define _ENABLE_DEBUG_

	bool	DEBUG_Init		(IN ARGB _color);
	bool	DEBUG_Uninit	();

	void	DEBUG_Clear		(IN ARGB _color);
	void	DEBUG_Message	(IN string* _message, IN ARGB _color);
	void	DEBUG_Data		(IN string* _message, IN dword _data, IN ARGB _color);
	dword	DEBUG_EIP		();
	void	DEBUG_Reset		();

#ifdef _ENABLE_DEBUG_
	#define DEBUG(X)			{string message = STRING(X); DEBUG_Message(&message, 0x0000FF00);}
	#define DEBUG_DATA(X, Y, Z)	{string message = STRING(X); DEBUG_Data(&message, (Y), (Z));}
#else
	#define DEBUG(X)
	#define DEBUG_DATA(X, Y, Z)
#endif

#endif
