#include <iostream>
#include <string>
#include <sstream>

#include "XkyAdapter.h"
#include "Disk.h"
#include "XFS_EntryWriter.h"
#include "XFS_FileWriter.h"
#include "XFS_DirectoryWriter.h"
#include "MappedFile.h"

#define BOOT_NAME	"Boot"
#define LOADER_NAME	"Loader"

u32 ToNumber(std::string number)
{
	std::stringstream ss(std::ios::in | std::ios::out);
	ss<<number;
	u32 data;
	ss>>data;
	return data;
}
int main(int argc, char* argv[])
{
	try
	{
		//Parameters
		if(argc!=3 && argc!=4)
		{
			std::cout<<"XFS <path> <disk> [<disk size>]"<<std::endl;
			return 0;
		}

		std::string windows_path=argv[1];
		std::string xfs_disk=argv[2];
		u32 xfs_disk_size=(argc==4?ToNumber(argv[3]):FLOPPY_SIZE);

		//Instanciamos el disco
		Disk* disk=Disk::GetDisk(xfs_disk, xfs_disk_size);
		
		//Variables del disco
		OFFSET xft_entry=TO_OFFSET(XFT_LBA);
		LBA writing_sector=XFT_LBA+XFT_SECTORS;

		//Escribimos el sector de arranque
		MappedFile boot_file(windows_path + "\\BOOT\\" BOOT_NAME);
		disk->Write(0, boot_file.GetBasePointer(), SECTOR_SIZE);

		//Escribimos el loader 
		//MappedFile loader_file(windows_path + "\\BOOT\\" LOADER_NAME);
		writing_sector+=XFS_FileWriter::Write(disk, windows_path + "\\BOOT", LOADER_NAME, xft_entry, writing_sector);
		xft_entry+=XFS_ENTRY_SIZE;

		//Escribimos el kernel
		writing_sector+=XFS_DirectoryWriter::Write(disk, windows_path, "KRNL", true, xft_entry, writing_sector, XFT_LBA);
		xft_entry+=XFS_ENTRY_SIZE;
		
		//Escribimos XFS
		writing_sector+=XFS_DirectoryWriter::Write(disk, windows_path, "XFS", false, xft_entry, writing_sector, XFT_LBA);
		xft_entry+=XFS_ENTRY_SIZE;

		delete disk;
	}
	catch(char* error)
	{
		std::cout<<error<<std::endl;
	}
	catch(std::string error)
	{
		std::cout<<error.c_str()<<std::endl;
	}
	catch(...)
	{
		std::cout<<"Unexpected error!"<<std::endl;
	}
	return 0;
}
