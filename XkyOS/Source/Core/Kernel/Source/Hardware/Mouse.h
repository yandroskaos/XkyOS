/******************************************************************************/
/**
* @file		Mouse.h
* @brief	XkyOS Hardware Mouse Library
* Definitions of mouse operations and types
* 
* @date		20/03/2008
* @author	Pablo Bravo
*/
/******************************************************************************/
#ifndef __MOUSE_H__
#define __MOUSE_H__

	bool	MOUSE_Init();

	dword	MOUSE_GetX();
	dword	MOUSE_GetY();
	bool	MOUSE_IsLeftButtonDown();
	bool	MOUSE_IsRightButtonDown();

#endif //__MOUSE_H__
