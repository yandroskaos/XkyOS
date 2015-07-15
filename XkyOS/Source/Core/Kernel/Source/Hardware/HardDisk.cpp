/******************************************************************************/
/**
* @file		HardDisk.cpp
* @brief	XkyOS Hardware Hard Disk Library
* Implementation of hard disk operations and types
* 
* @date		20/03/2008
* @author	Pablo Bravo
*/
/******************************************************************************/
#include "Types.h"
#include "System.h"
#include "Memory.h"
#include "IO.h"
#include "HardDisk.h"

//==================================DATA======================================//
#pragma data_seg(".data")
//============================================================================//
PRIVATE dword hd_last_sector = 0;
PRIVATE dword hd_boot_drive = 0;

#define FLOPPY_RAM_DISK	0x00040000
#define FLOPPY_SIZE	1474560

//==================================CODE======================================//
#pragma code_seg(".code")
//============================================================================//
/**
* @brief Copies _size bytes from _origin to _destiny.
* @param _destiny [in] Destiny address.
* @param _origin [in] Source addres.
* @param _size [in] Bytes to copy.
*/
PRIVATE NAKED void MEM_Copy(IN VIRTUAL _destiny, IN VIRTUAL _origin, IN dword _size)
{
	__asm
	{
		push ebp
		mov ebp, esp
		push ecx
		push esi
		push edi
		mov ecx, dword ptr [ebp + 16] //_size
		mov esi, dword ptr [ebp + 12] //_origin
		mov edi, dword ptr [ebp + 8]  //_destiny
		rep movsb
		pop edi
		pop esi
		pop ecx
		pop ebp
		ret 12
	}
}

/**
* @brief Hard Disk initialization.
* @param _loader_data [in] Loader data regarding disk.
* @return Returns true if everything goes well.
*/
PUBLIC bool HD_Init(IN DISK_LOADER_DATA* _loader_data)
{
	hd_boot_drive = _loader_data->boot_drive;
	if(hd_boot_drive == (dword)0x80)
	{
		//HARD-DISK
		//hd_last_sector = _loader_data->boot_drive_size;
		hd_last_sector = 0x0FFFFFFF; //Temporary
	}
	else
	{
		//FLOPPY
		hd_last_sector = 512;
	}
	
	return true;
}

/**
* @brief Check if addresses exists.
* @param _lba [in] A starting lba.
* @param _number_of_sectors [in] Number of contiguos sectors.
* @return Returns first page entry if enough pages are found.
*/
PRIVATE bool HD_CheckLBA(IN LBA _lba, IN dword _number_of_sectors)
{
	return ((_lba + _number_of_sectors) < hd_last_sector);
}

/**
* @brief Prepare Hard Disk for action.
* @param _lba [in] LBA regarding operation.
*/
PRIVATE void HD_StartHD(IN byte _drive, IN LBA _lba)
{
	//Send a NULL byte to port 0x1F1
	IO_OutPortByte(0x1F1, 0x00);

	//Send a sector count to port 0x1F2
	IO_OutPortByte(0x1F2, 0x01);

	//Send the low 8 bits of the block address to port 0x1F3
	IO_OutPortByte(0x1F3, (byte)_lba);

	//Send the next 8 bits of the block address to port 0x1F4
	IO_OutPortByte(0x1F4, (byte)(_lba >> 8));

	//Send the next 8 bits of the block address to port 0x1F5
	IO_OutPortByte(0x1F5, (byte)(_lba >> 16));
	
	//Send the drive indicator, some magic bits, and highest 4 bits of the block address to port 0x1F6
	IO_OutPortByte(0x1F6, 0xE0 | (_drive << 4) | (((byte)(_lba >> 24)) & 0x0F));
}

/**
* @brief Waits for hard disk to be prepared.
*/
PRIVATE void HD_WaitForHD()
{
	while(!(IO_InPortByte(0x1F7) & 0x08));
}

/**
* @brief Reads one sector from hard disk.
* @param _disk [in] Disk to read from.
* @param _lba [in] LBA to be read.
* @param _buffer [out] Where to leave the data.
* @return Returns true if sector was read.
*/
PRIVATE bool HD_ReadSector(IN byte _disk, IN LBA _lba, OUT VIRTUAL _buffer)
{
	//Prepare the HD
	HD_StartHD(_disk, _lba);
	
	//Send the command (0x20) to port 0x1F7
	IO_OutPortByte(0x1F7, 0x20);

	//Wait
	HD_WaitForHD();
	
	//Read
	for(dword i = 0; i < SECTOR_SIZE; i+=4)
	{
		*((dword*)(_buffer + i)) = IO_InPortDword(0x1F0);
	}
	
	//Done
	return true;
}

/**
* @brief Reads one sector from floppy.
* @param _disk [in] Disk to read from.
* @param _lba [in] LBA to be read.
* @param _buffer [out] Where to leave the data.
* @return Returns true if sector was read.
*/
PRIVATE bool FLOPPY_ReadSector(IN byte _disk, IN LBA _lba, OUT VIRTUAL _buffer)
{
	MEM_Copy(_buffer, (VIRTUAL)(FLOPPY_RAM_DISK + _lba*SECTOR_SIZE), SECTOR_SIZE);
	return true;
}

/**
* @brief Reads _number_of_sectors sectors.
* @param _disk [in] Disk to read from.
* @param _lba [in] LBA to be read.
* @param _number_of_sectors [in] Number of sectors to read.
* @param _buffer [out] Where to leave the data.
* @return Returns the number of sectors read from the begining.
*/
PUBLIC dword HD_ReadSectors(IN byte _disk, IN LBA _lba, IN dword _number_of_sectors, OUT VIRTUAL _buffer)
{
	//Check
	if(!HD_CheckLBA(_lba, _number_of_sectors))
		return 0;
	
	//Read
	if(hd_boot_drive == (dword)0x80)
	{
		for(dword i = 0; i < _number_of_sectors; i++)
		{
			if(!HD_ReadSector(0 /*_disk*/, _lba + i, _buffer + i*SECTOR_SIZE))
				return i;
		}
	}
	else
	{
		for(dword i = 0; i < _number_of_sectors; i++)
		{
			if(!FLOPPY_ReadSector(0 /*_disk*/, _lba + i, _buffer + i*SECTOR_SIZE))
				return i;
		}
	}

	//Ok
	return _number_of_sectors;
}

/**
* @brief Writes one sector from hard disk.
* @param _disk [in] Disk to write to.
* @param _lba [in] LBA to be written.
* @param _buffer [in] Where to get the data from.
* @return Returns true if sector was read.
*/
PRIVATE bool HD_WriteSector(IN byte _disk, IN LBA _lba, IN VIRTUAL _buffer)
{
	//Preparamos el HD
	HD_StartHD(_disk, _lba);
	
	//Send the command (0x30) to port 0x1F7
	IO_OutPortByte(0x1F7, 0x30);

	//Wait
	HD_WaitForHD();

	//Write
	for(dword i = 0; i < SECTOR_SIZE; i+=4)
	{
		IO_OutPortDword(0x1F0, *(dword*)(_buffer + i));
	}

	//Done
	return true;
}

/**
* @brief Writes one sector from floppy.
* @param _disk [in] Disk to write to.
* @param _lba [in] LBA to be written.
* @param _buffer [in] Where to get the data from.
* @return Returns true if sector was read.
*/
PRIVATE bool FLOPPY_WriteSector(IN byte _disk, IN LBA _lba, IN VIRTUAL _buffer)
{
	MEM_Copy((VIRTUAL)(FLOPPY_RAM_DISK + _lba*SECTOR_SIZE), _buffer, SECTOR_SIZE);
	return true;
}

/**
* @brief Writes _number_of_sectors sectors.
* @param _disk [in] Disk to write to.
* @param _lba [in] LBA to be written.
* @param _number_of_sectors [in] Number of sectors to write.
* @param _buffer [out] Where to get the data.
* @return Returns the number of sectors written from the begining.
*/
PUBLIC dword HD_WriteSectors(IN byte _disk, IN LBA _lba, IN dword _number_of_sectors, IN VIRTUAL _buffer)
{
	//Check
	if(!HD_CheckLBA(_lba, _number_of_sectors))
		return false;

	//Write
	if(hd_boot_drive == (dword)0x80)
	{
		for(dword i = 0; i < _number_of_sectors; i++)
		{
			if(!HD_WriteSector(0 /*_disk*/, _lba + i, _buffer + i*SECTOR_SIZE))
				return i;
		}
	}
	else
	{
		for(dword i = 0; i < _number_of_sectors; i++)
		{
			if(!FLOPPY_WriteSector(0 /*_disk*/, _lba + i, _buffer + i*SECTOR_SIZE))
				return i;
		}
	}

	//Ok
	return _number_of_sectors;
}

/**
* @brief Consults disk size.
* @return Returns the size IN SECTORS of the disk.
*/
dword HD_Size()
{
	return hd_last_sector;
}

/**
* @brief Consults the boot drive.
* @return Returns the boot drive.
*/
dword HD_BootDrive()
{
	return hd_boot_drive;
}