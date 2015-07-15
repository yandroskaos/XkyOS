/******************************************************************************/
/**
* @file		Loader.h
* @brief	Loader parameters from real-mode start
* 
* @date		20/03/2008
* @author	Pablo Bravo
*/
/******************************************************************************/
#ifndef __LOADER_H__
#define __LOADER_H__

	#include "Types.h"

	/**
	* @brief Describes all data related to disk, including XFT address, KRNL directory in memory and boot parameters.
	*/
	struct DISK_LOADER_DATA
	{
		dword xft;
		dword krnl_directory;
		dword boot_drive;
		dword boot_drive_size;
	};

	/**
	* @brief Describes a memory range.
	*/
	struct MEMORY_DESCRIPTOR
	{
		dword base_address_low;
		dword base_address_high;
		dword base_length_low;
		dword base_length_high;
		dword type;
	};

	/**
	* @brief Definition of memory data passed from real mode.
	*/
	struct MEMORY_LOADER_DATA
	{
		dword size_in_megas;
		dword descriptors_number;
		MEMORY_DESCRIPTOR descriptors[];
	};

	#define MEMORY_TYPE_AVAILABLE		0x00000001 /**< Indicates the memory range is available */
	#define MEMORY_TYPE_SYSTEM_RESERVED	0x00000002 /**< Indicates the memory range is reserved (ROM, BIOS, etc) */

	/**
	* @brief Definition of video data passed from real mode.
	*/
	struct SVGA_LOADER_DATA
	{
		dword framebuffer;
		dword x_resolution;
		dword y_resolution;
	};

	/**
	* @brief Definition of all data passed from real mode.
	* Compresses all data read in real-mode. Has parameters about the hard disk, the memory system
	* and the video system.
	*/
	struct LOADER_DATA
	{
		DISK_LOADER_DATA*	disk;
		MEMORY_LOADER_DATA*	memory;
		SVGA_LOADER_DATA*	svga;
	};

#endif //__LOADER_H__
