/******************************************************************************/
/**
* @file		Image.h
* @brief	XkyOS basic image files types and definitions
* This file contains the basic definitions used in XkyOS for it's image files (executables).
* A XkyOS image file is formed of sections. The file has a module header made of a magic value, a file header,
* and a fixed number of section headers.
* 
* @date		20/03/2008
* @author	Pablo Bravo
*/
/******************************************************************************/
#ifndef __IMAGE_H__
#define __IMAGE_H__

#include "Types.h"

/**
* @brief	XkyOS image section header
* This defines a section within an executable image.
*/
struct IMG_SECTION_HEADER
{
	byte	name[4];
	dword	offset;
	dword	size;
	dword	flags;
};

/**
* @brief	XkyOS image file header
* Has information about the whole image, such as entry point, etc.
*/
struct IMG_FILE_HEADER
{
	dword	size;
	dword	version;
	dword	kind;
	dword	mode;
	dword	init_ep;
	dword	code_ep;
	dword	finish_ep;
};

#define IMAGE_SIGNATURE 'XYKX' /*< The valid module signature*/

/**
* @brief	XkyOS image module header
* Represents all the header needed in an executable.
*/
struct IMG_MODULE_HEADER
{
	dword				signature;
	IMG_FILE_HEADER		file_header;
	IMG_SECTION_HEADER	imports_section;
	IMG_SECTION_HEADER	data_section;
	IMG_SECTION_HEADER	code_section;
	IMG_SECTION_HEADER	exports_section;
	IMG_SECTION_HEADER	relocs_section;
};

#define IMAGE_SECTION_CODE		0x00000001	/*< The section has code*/
#define IMAGE_SECTION_DATA		0x00000002	/*< The section has data*/
#define IMAGE_MODE_USER			0x00000001	/*< The image is for user-land*/
#define IMAGE_MODE_SYSTEM		0x00000000	/*< The image is for kernel-land*/
#define IMAGE_KIND_CLASS		0x00000001	/*< The image is a class-based module (unsupported)*/
#define IMAGE_KIND_MODULE		0x00000002	/*< The image is a function-based module*/

/**
* @brief	Allow to create a 4 byte version.
*/
#define IMAGE_VERSION(w,x,y,z)	((w<<24)|(x<<16)|(y<<8)|(z))

/**
* @brief	XkyOS image relocation.
* It's the first element in a "reloc" section of an image. Says how many relocations has an image, and
* where to apply them.
*/
struct IMG_RELOCATION
{
	dword number_of_relocations;
	dword relocations[];
};

/**
* @brief	Maximum size for bounded string used in exports. Allows extern's to be 64 bytes.
*/
#define MAX_EXTERN_NAME_SIZE 59

/**
* @brief	XkyOS image import.
* Imports work in XkyOS by name. So an import it's made by the name of the function and a place to 
* put the pointer value.
*/
struct IMG_IMPORT
{
	DEFINE_BOUNDED_STRING(name, MAX_EXTERN_NAME_SIZE);
	dword function;
};

/**
* @brief	XkyOS image import.
* Exports work in XkyOS by name. So an import it's made by the name of the function exported and
* it's current address.
*/
struct IMG_EXPORT
{
	DEFINE_BOUNDED_STRING(name, MAX_EXTERN_NAME_SIZE);
	dword function;
};

#endif //__IMAGE_H__
