/******************************************************************************/
/**
* @file		HardDisk.h
* @brief	XkyOS Hardware Hard Disk Library
* Definitions of hard disk operations and types
* 
* @date		20/03/2008
* @author	Pablo Bravo
*/
/******************************************************************************/
#ifndef __HARDDISK_H__
#define __HARDDISK_H__

	#include "Loader.h"
	#include "Memory.h"

	/**
	* @brief Disk addresses type (Logical Block Address).
	*/
	typedef dword LBA;

	bool	HD_Init(IN DISK_LOADER_DATA* _loader_data);

	dword	HD_ReadSectors	(IN byte _disk, IN LBA _lba, IN dword _sectors, OUT VIRTUAL _buffer);
	dword	HD_WriteSectors	(IN byte _disk, IN LBA _lba, IN dword _sectors, IN VIRTUAL _buffer);
	dword	HD_Size			();

	#define FLOPPY_DRIVE	0
	#define HD_DRIVE		0x00000080
	dword	HD_BootDrive	();

#endif //__HARDDISK_H__
