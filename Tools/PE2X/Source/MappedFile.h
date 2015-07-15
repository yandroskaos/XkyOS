#ifndef __MAPPED_FILE_H__
#define __MAPPED_FILE_H__

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <string>

class MappedFile
{
private:
	void* file;
	void* mapped_file;
	void* file_base_pointer;
	DWORD file_size;

public:
	MappedFile(const std::string& file_name);
	~MappedFile();

	BYTE* GetBasePointer();
	DWORD GetSize();
};


#endif
