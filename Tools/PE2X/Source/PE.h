#ifndef __PE_H__
#define __PE_H__

#include <windows.h>
#include <string>
#include "MappedFile.h"

class PEFile
{
	MappedFile* file;
	IMAGE_NT_HEADERS* nt_headers;
	IMAGE_SECTION_HEADER* sections;

public:
	PEFile(std::string file_name);

	IMAGE_SECTION_HEADER* GetSectionHeaderByIndex(DWORD index);
	BYTE* GetSectionByIndex(DWORD index);
	DWORD NumberOfSections();

	IMAGE_SECTION_HEADER* GetSectionHeaderByName(const char* name);
	BYTE* GetSectionByName(const char* name);
	
	IMAGE_SECTION_HEADER* GetSectionHeaderByRVA(DWORD rva);
	BYTE* GetSectionByRVA(DWORD rva);

	BYTE* Raw();
	DWORD Size();
};

#endif