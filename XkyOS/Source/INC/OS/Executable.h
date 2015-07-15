/******************************************************************************/
/**
* @file		Executable.h
* @brief	XkyOS basic macros for binary executable generation.
* 
* @date		20/03/2008
* @author	Pablo Bravo
*/
/******************************************************************************/
#ifndef __EXECUTABLE_H__
#define __EXECUTABLE_H__

#include "Types.h"
#include "Image.h"

/**
* @brief	Generates binay image header in an executable. Needed for compilation.
*/
#define MODULE(_Mode, _Kind, _Version, _InitEP, _CodeEP, _FinishEP)	IMG_MODULE_HEADER _module={IMAGE_SIGNATURE, \
		{0, _Version, _Kind, _Mode, (dword)_InitEP, (dword)_CodeEP, (dword)_FinishEP}, \
		{{'i','m','p','s'}, 0, 0, IMAGE_SECTION_DATA},	\
		{{'d','a','t','a'}, 0, 0, IMAGE_SECTION_DATA},	\
		{{'c','o','d','e'}, 0, 0, IMAGE_SECTION_CODE},	\
		{{'e','x','p','s'}, 0, 0, IMAGE_SECTION_DATA},	\
		{{'r','e','l','c'}, 0, 0, IMAGE_SECTION_DATA}}

/**
* @brief	Generates binay image import. Needed for compilation.
*/
#define IMPORT(X)	ALIGN(1) byte X##_size = sizeof(#X)-1 ; \
					ALIGN(1) byte X##_text[MAX_EXTERN_NAME_SIZE] = #X; \
					ALIGN(1) f##X X = (f##X)0x00000000

/**
* @brief	Generates binay image export. Needed for compilation.
*/
#define EXPORT(X)	IMG_EXPORT _##X = {sizeof(#X)-1 , #X, (dword)X}

#endif
