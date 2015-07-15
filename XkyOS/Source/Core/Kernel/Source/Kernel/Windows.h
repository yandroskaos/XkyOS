/******************************************************************************/
/**
* @file		Windows.h
* @brief	XkyOS Window manager
* Definitions of window subsystem
* 
* @date		20/03/2008
* @author	Pablo Bravo
*/
/******************************************************************************/
#ifndef __WINDOWS_H__
#define __WINDOWS_H__

	#include "Types.h"
	#include "SVGA.h"
	#include "AddressSpace.h"

	/**
	* @brief Window handle type.
	*/	
	typedef dword WINDOW;
	/**
	* @brief The mouse callback type.
	*/
	typedef void (*fWindowMouseCallback)(IN dword _x, IN dword _y, IN bool _left_clicked, IN bool _right_clicked);
	/**
	* @brief The keyboard callback type.
	*/
	typedef void (*fWindowKeyboardCallback)(IN dword _scan_code);

	bool WINDOW_Init();

	WINDOW	WINDOW_GetWindow		();
	void	WINDOW_ReleaseWindow	(IN WINDOW _window);
	
	dword	WINDOW_GetHeight(IN WINDOW _window);
	dword	WINDOW_GetWidth	(IN WINDOW _window);
	ARGB	WINDOW_GetPixel	(IN WINDOW _window, IN dword _x, IN dword _y);
	void	WINDOW_SetPixel	(IN WINDOW _window, IN dword _x, IN dword _y, IN ARGB _color);
	void	WINDOW_PrintText(IN WINDOW _window, IN dword _x, IN dword _y, IN ARGB _color, IN string* _text);

	void	WINDOW_RegisterKeyboard	(IN WINDOW _window, IN ADDRESS_SPACE _pdbr, IN fWindowKeyboardCallback _keyboard_callback);
	void	WINDOW_RegisterMouse	(IN WINDOW _window, IN ADDRESS_SPACE _pdbr, IN fWindowMouseCallback _mouse_callback);

	/**
	* @brief Pointer handle type.
	*/	
	typedef dword POINTER;
	POINTER	POINTER_GetArrow();
	POINTER	POINTER_GetClock();

	void	WINDOW_SetPointer		(IN WINDOW _window, IN POINTER _pointer);
	void	WINDOW_SetPointerColor	(IN WINDOW _window, IN ARGB _color);

	//Private kernel use
	void WINDOW_FlushKeyboardCallbacks	(IN WINDOW _window);
	void WINDOW_FlushMouseCallbacks		(IN WINDOW _window);

#endif
