#include "PE.h"

PEFile::PEFile(std::string file_name)
{
	file = new MappedFile(file_name);
	if(!file)
		throw std::string("[PEFile] Could not map file: ")+file_name;

	//Creamos las estructuras de datos necesarias
	IMAGE_DOS_HEADER* dos_header = (IMAGE_DOS_HEADER*)(file->GetBasePointer());
	
	nt_headers	= (IMAGE_NT_HEADERS*)		(file->GetBasePointer() + dos_header->e_lfanew);
	sections	= (IMAGE_SECTION_HEADER*)	((BYTE*)nt_headers + sizeof(IMAGE_NT_HEADERS));
}

IMAGE_SECTION_HEADER* PEFile::GetSectionHeaderByIndex(DWORD index)
{
	if(index >= nt_headers->FileHeader.NumberOfSections)
		return 0;

	return sections + index;
}

BYTE* PEFile::GetSectionByIndex(DWORD index)
{
	IMAGE_SECTION_HEADER* section_header = GetSectionHeaderByIndex(index);
	if(!section_header) return 0;

	return file->GetBasePointer() + section_header->PointerToRawData;
}

DWORD PEFile::NumberOfSections()
{
	return (DWORD)nt_headers->FileHeader.NumberOfSections;
}

IMAGE_SECTION_HEADER* PEFile::GetSectionHeaderByName(const char* name)
{
	if(strlen(name)-1 > 8) return 0;

	WORD i;
	for(i=0; i<nt_headers->FileHeader.NumberOfSections; i++)
	{
		if(memcmp(sections[i].Name, name, strlen(name)-1) == 0)
			return sections + i;
	}
	return 0;
}

BYTE* PEFile::GetSectionByName(const char* name)
{
	IMAGE_SECTION_HEADER* section_header = GetSectionHeaderByName(name);
	if(!section_header) return 0;

	return file->GetBasePointer() + section_header->PointerToRawData;
}

IMAGE_SECTION_HEADER* PEFile::GetSectionHeaderByRVA(DWORD rva)
{
	WORD i;
	for(i=0; i<nt_headers->FileHeader.NumberOfSections; i++)
	{
		if((rva>=sections[i].VirtualAddress) && (rva<=(sections[i].VirtualAddress + sections[i].Misc.VirtualSize)))
			return sections + i;
	}

	return 0;
}

BYTE* PEFile::GetSectionByRVA(DWORD rva)
{
	IMAGE_SECTION_HEADER* section_header = GetSectionHeaderByRVA(rva);
	if(!section_header) return 0;

	return file->GetBasePointer() + section_header->PointerToRawData;
}

BYTE* PEFile::Raw()
{
	return file->GetBasePointer();
}

DWORD PEFile::Size()
{
	return file->GetSize();
}
