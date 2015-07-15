#include "MappedFile.h"

#define WIN32_LEAN_AND_MEAN
#include <windows.h>

MappedFile::MappedFile(const std::string& file_name)
{
	file=CreateFile(file_name.c_str(), GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_READONLY|FILE_FLAG_RANDOM_ACCESS, NULL);
	if(file==INVALID_HANDLE_VALUE)
	{
		throw std::string("[MappedFile] CreateFile ")+file_name;
	}

	file_size=GetFileSize(file, NULL);

	mapped_file=CreateFileMapping(file, NULL, PAGE_READONLY, 0, 0, NULL);
	if(!mapped_file)
	{
		CloseHandle(file);
		throw "[MappedFile] CreateFileMapping";
	}

	file_base_pointer=MapViewOfFile(mapped_file, FILE_MAP_READ, 0, 0, 0);
	if(!file_base_pointer)
	{
		CloseHandle(mapped_file);
		CloseHandle(file);
		throw "[MappedFile] MapViewOfFile";
	}
}

MappedFile::~MappedFile()
{
	UnmapViewOfFile(file_base_pointer);
	CloseHandle(mapped_file);
	CloseHandle(file);
}

unsigned char* MappedFile::GetBasePointer()
{
	return static_cast<unsigned char*>(file_base_pointer);
}

unsigned int MappedFile::GetSize()
{
	return file_size;
}
