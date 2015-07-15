#ifndef __X_H__
#define __X_H__

#include <string>

#include "Types.h"
#include "Image.h"

class XFile
{
	union
	{
		byte* file;
		IMG_MODULE_HEADER* header;
	};
	
	dword write_pointer;
	dword file_size;

	dword AlignTo(dword value, dword alignment);

public:
	XFile(dword size);
	~XFile();

	bool SequentialWrite(void* what, dword size, dword alignment = 0);
	bool Write(dword offset, void* what, dword size);
	dword CurrentOffset();
	bool FlushToDisk(std::string file_name);

	IMG_SECTION_HEADER* GetSectionHeaderByName(const char* section_name);
	byte* GetSectionByName(const char* section_name);

	byte* Raw();
	dword Size();
	IMG_MODULE_HEADER* Header();
};

#endif
