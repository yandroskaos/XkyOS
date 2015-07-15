/******************************************************************************/
/**
* @file		XFS.h
* @brief	XkyOS File System
* Definitions of the minimal file system needed in XkyOS for booting
* 
* @date		20/03/2008
* @author	Pablo Bravo
*/
/******************************************************************************/
#ifndef __XFS_H__
#define __XFS_H__

#include "Types.h"

/**
* @brief XFT Disk Address.
*/
#define XFT_LBA		1
/**
* @brief Sectors ocuppied by XFT.
*/
#define XFT_SECTORS	1
/**
* @brief Size in bytes of the XFT.
*/
#define XFT_SIZE	(SECTOR_SIZE*XFT_SECTORS)

/**
* @brief Number of sectors per directory.
*/
#define XFS_DIRECTORY_SECTORS	8
/**
* @brief Size in bytes of a directory.
*/
#define XFS_DIRECTORY_SIZE		(SECTOR_SIZE*XFS_DIRECTORY_SECTORS)

/**
* @brief Number of characters per file name.
*/
#define XFS_ENTRY_NAME_SIZE		52

/**
* @brief Xky File System Entry.
*/
struct XFS_ENTRY
{
	byte	used;
	byte	is_directory;
	byte	items;
	DEFINE_BOUNDED_STRING(name, XFS_ENTRY_NAME_SIZE);
	dword	direction;
	dword	size;
};

#endif //__XFS_H__
