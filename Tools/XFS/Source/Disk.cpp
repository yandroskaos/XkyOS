#include "Disk.h"

//FloppyDisk
#include <windows.h>
#define DRIVE_A "\\\\.\\A:"

class FloppyDisk : public Disk
{
	u8 disk[FLOPPY_SIZE];

	void CheckOffset(OFFSET offset, u32 size)
	{
		if(offset+size-1>FLOPPY_SIZE)
			throw std::string("FloppyDisk::CheckOffset");
	}
public:
	FloppyDisk()
	{
		//Ponemos a cero el buffer
		memset(disk, 0, FLOPPY_SIZE);
	}
	~FloppyDisk()
	{
		//Escribimos a disquete
		HANDLE file=CreateFile(DRIVE_A, GENERIC_WRITE, FILE_SHARE_READ, 0, OPEN_EXISTING, 0, 0);
		DWORD bytes_written;
		if(!WriteFile(file, disk, FLOPPY_SIZE, &bytes_written, 0))
			throw std::string("[FloppyDisk] Writing");
	}

	void Write(OFFSET offset, u8* data, u32 size)
	{
		CheckOffset(offset, size);

		memcpy(disk+offset, data, size);
	}

	void Read(OFFSET offset, u8* data, u32 size)
	{
		CheckOffset(offset, size);

		memcpy(data, disk+offset, size);
	}
};

//File Disk
#include <cstdio>

class FileDisk : public Disk
{
	FILE* file;
	u32 file_size;

	u32 CalculateFileSize(std::string file_name)
	{
		HANDLE f;
		LARGE_INTEGER s;
		
		f=CreateFile(file_name.c_str(), GENERIC_READ, FILE_SHARE_READ, 0, OPEN_EXISTING, 0, 0);
		if(f==INVALID_HANDLE_VALUE)
			throw std::string("[FileDisk]CreateFile");
		
		if(!GetFileSizeEx(f, &s))
			throw std::string("[FileDisk]GetFileSizeEx");
		CloseHandle(f);

		return s.LowPart;
	}
	void CheckOffset(OFFSET offset, u32 size)
	{
		if(offset+size-1>file_size)
			throw std::string("FileDisk::CheckOffset");
	}

	void MoveFilePointer(OFFSET offset)
	{
		if(fseek(file, offset, SEEK_SET))
			throw std::string("FileDisk::MoveFilePointer");
	}

public:
	FileDisk(std::string file_name, u32 desired_size)
	{
		//Chequeamos si existe y si no lo cremaos
		file=fopen(file_name.c_str(), "rb");
		if(!file)
		{
			//Creamos
			file=fopen(file_name.c_str(), "wb");

			//Relenamos con ceros
			u8 null=0;
			for(unsigned int i=0; i<desired_size; i++)
				fwrite(&null, 1, 1, file);
		}
		fclose(file);

		//Obtenemos el file size
		file_size=CalculateFileSize(file_name);

		//Abrimos
		file=fopen(file_name.c_str(), "r+b");
	}
	~FileDisk()
	{
		fclose(file);
	}

	void Write(OFFSET offset, u8* data, u32 size)
	{
		CheckOffset(offset, size);
		MoveFilePointer(offset);
		fwrite(data, size, 1, file);
	}

	void Read(OFFSET offset, u8* data, u32 size)
	{
		CheckOffset(offset, size);
		MoveFilePointer(offset);
		fread(data, size, 1, file);
	}
};

//Disk
Disk* Disk::GetDisk(std::string disk_name, u32 desired_size)
{
	if(disk_name==FLOPPY_DISK_NAME)
	{
		return new FloppyDisk;
	}
	else
	{
		return new FileDisk(disk_name, desired_size);
	}
}
