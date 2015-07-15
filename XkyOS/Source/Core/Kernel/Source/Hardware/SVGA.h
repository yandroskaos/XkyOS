/******************************************************************************/
/**
* @file		SVGA.h
* @brief	XkyOS Graphic Screen Library
* Definitions of SVGA operations and types
* 
* @date		20/03/2008
* @author	Pablo Bravo
*/
/******************************************************************************/
#ifndef __SVGA_H__
#define __SVGA_H__

	#include "Loader.h"

	/**
	* @brief Represents a color.
	*/
	typedef dword ARGB;

	/**
	* @brief Creates a solid color.
	*/
	#define SRGB(R,G,B) ((ARGB)((0x00<<24)|((R)<<16)|((G)<<8)|((B)))) //ARGB SolidRGB(IN byte _R, IN byte _G, IN byte _B);
	/**
	* @brief Creates a transparent color.
	*/
	#define TRGB(R,G,B) ((ARGB)((0xFF<<24)|((R)<<16)|((G)<<8)|((B)))) //ARGB TransparentRGB(IN byte _R, IN byte _G, IN byte _B);

	/**
	* @brief Gets the red component of a color.
	*/
	#define RED(C)	((C)&0x00FF0000) //ARGB RED(IN ARGB _color);
	/**
	* @brief Gets the green component of a color.
	*/
	#define GREEN(C)((C)&0x0000FF00) //ARGB GREEN(IN ARGB _color);
	/**
	* @brief Gets the blue component of a color.
	*/
	#define BLUE(C)	((C)&0x000000FF) //ARGB BLUE(IN ARGB _color);

	/**
	* @brief Gets a solid color from other.
	*/
	#define COLOR_MakeSolid(C)			((C) & 0x00FFFFFF)				//ARGB COLOR_MakeSolid(IN ARGB _color);
	/**
	* @brief Gets a transparent color from other.
	*/
	#define COLOR_MakeTransparent(C)	((C) | 0xFF000000)				//ARGB COLOR_MakeTransparent(IN ARGB _color);
	/**
	* @brief Tells if a given color is solid.
	*/
	#define COLOR_IsSolid(C)			(!COLOR_IsTransparent(C))		//bool COLOR_IsSolid(IN ARGB _color);
	/**
	* @brief Tells if a given color is transparent.
	*/
	#define	COLOR_IsTransparent(C)		(((C) & 0xFF000000)?true:false)	//bool COLOR_IsTransparent(IN ARGB _color);

	bool	SVGA_Init(IN SVGA_LOADER_DATA* _loader_data);

	bool	SVGA_IsGraphicModeEnabled	();
	dword	SVGA_GetWidth				();
	dword	SVGA_GetHeight				();
	
	ARGB	SVGA_GetPixel		(IN dword _x, IN dword _y);
	void	SVGA_SetPixel		(IN dword _x, IN dword _y, IN ARGB _color);
	void	SVGA_ClearScreen	(IN ARGB _color);
	void	SVGA_PrintCharacter	(IN dword _x, IN dword _y, IN ARGB _color, IN byte _character);
	void	SVGA_PrintText		(IN dword _x, IN dword _y, IN ARGB _color, IN string* _text);
	
#endif //__SVGA_H__
