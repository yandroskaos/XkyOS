/******************************************************************************/
/**
* @file		DiskRange.cpp
* @brief	XkyOS Disk Arrangement Library
* 
* @date		20/03/2008
* @author	Pablo Bravo
*/
/******************************************************************************/
#include "DiskRange.h"

//==================================CODE======================================//
#pragma code_seg(".code")
//============================================================================//
/**
* @brief Fills a disk range.
* @param _range [out] The disk range to fill.
* @param _start_sector [in] The sector we start the allocation from.
* @param _number_of_sectors [in] The size in sectors.
*/
PUBLIC void DISK_RANGE_Fill(OUT DISK_RANGE& _range, IN LBA _start_sector, IN dword _number_of_sectors)
{
	_range.start_sector = _start_sector;
	_range.number_of_sectors = _number_of_sectors;
}


/**
* @brief Indicates if a given disk address is contained within a disk range.
* @param _range [in] The disk range we want to test against.
* @param _sector [in] The disk address to test.
* @return True if the _sector is contained in the _range.
*/
PUBLIC bool DISK_RANGE_Contains(IN DISK_RANGE _range, IN LBA _sector)
{
	return (_sector>=_range.start_sector) && (_sector<(_range.start_sector + _range.number_of_sectors));
}

/**
* @brief Indicates if a given disk range contains another range.
* @param _range [in] The disk range we want to test if is container.
* @param _other [in] The disk range we want to test if is contained.
* @return True if _range contains _other, false otherwise.
*/
PRIVATE bool DISK_RANGE_ContainsCompletely(IN DISK_RANGE _range, IN DISK_RANGE _other)
{
	return (_range.start_sector<=_other.start_sector) && ((_range.start_sector + _range.number_of_sectors) >= (_other.start_sector + _other.number_of_sectors));
}

/**
* @brief Checks for any superposition between disk ranges.
* @param _range [in] The disk range we want to test against _other for a clash.
* @param _other [in] The disk range we want to test against _range for a clash.
* @return True if both range contains at least a sector in common, false otherwise.
*/
PUBLIC bool DISK_RANGE_ClashWith(IN DISK_RANGE _range, IN DISK_RANGE _other)
{
	return DISK_RANGE_Contains(_range, _other.start_sector) || DISK_RANGE_Contains(_range, _other.start_sector + _other.number_of_sectors - 1) || DISK_RANGE_ContainsCompletely(_other, _range);
}
