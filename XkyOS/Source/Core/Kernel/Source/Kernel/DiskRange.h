/******************************************************************************/
/**
* @file		DiskRange.h
* @brief	XkyOS Disk Arrangement Library
* 
* @date		20/03/2008
* @author	Pablo Bravo
*/
/******************************************************************************/
#ifndef __DISK_RANGE_H__
#define __DISK_RANGE_H__

	#include "Types.h"
	#include "HardDisk.h"

	/**
	* @brief Defines a range in the hard disk.
	* It is a set of disk addresses (LBA's) defined by the starting LBA and
	* the number of sectors used.
	*/
	struct DISK_RANGE
	{
		LBA		start_sector;
		dword	number_of_sectors;
	};

	void	DISK_RANGE_Fill	(OUT DISK_RANGE& _range, IN LBA _start_sector, IN dword _number_of_sectors);
	
	bool	DISK_RANGE_Contains	(IN DISK_RANGE _range, IN LBA _sector);
	bool	DISK_RANGE_ClashWith(IN DISK_RANGE _range, IN DISK_RANGE _other);

#endif //__DISK_RANGE_H__
