/******************************************************************************/
/**
* @file		Graphics.h
* @brief	Graphic definitions
* 
* @date		20/03/2008
* @author	Pablo Bravo
*/
/******************************************************************************/
#ifndef __GRAPHICS_H__
#define __GRAPHICS_H__

	#include "Types.h"

	/**
	* @brief Describes a point.
	*/
	struct COORDINATE
	{
		dword x;
		dword y;
	};

	/**
	* @brief Describes a rectangle, given two points.
	*/
	struct RECTANGLE
	{
		COORDINATE down_left;
		COORDINATE up_right;
	};

	/**
	* @brief Describes a size of a rectangle.
	*/
	struct SIZE
	{
		dword height;
		dword width;
	};

#endif //__GRAPHICS_H__
