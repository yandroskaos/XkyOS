#include "X.h"

dword XFile::AlignTo(dword value, dword alignment)
{
	if(value%alignment) value += (alignment - (value%alignment));
	return value;
}


XFile::XFile(dword size) : write_pointer(0), file_size(size)
{
	file = new byte[file_size];
	if(!file)
		throw std::string("[XFile] Cant allocate memory");
	memset(file, 0, file_size);
};

XFile::~XFile()
{
	delete[] file;
}

bool XFile::SequentialWrite(void* what, dword size, dword alignment)
{
	if(write_pointer + size > file_size)
		return false;

	memcpy(file + write_pointer, what, size);
	
	write_pointer+=size;
	if(alignment)
		write_pointer = AlignTo(write_pointer, alignment);

	return true;
}

bool XFile::Write(dword offset, void* what, dword size)
{
	if(offset + size > file_size)
		return false;
	
	memcpy(file + offset, what, size);
	return true;
}

#include <cstdio>
bool XFile::FlushToDisk(std::string file_name)
{
	FILE* out=fopen(file_name.c_str(), "wb");
	if(!out)
		return false;
	fwrite(file, 1, file_size, out);
	fclose(out);
	return true;
}

byte* XFile::Raw()
{
	return file;
}

dword XFile::Size()
{
	return file_size;
}
IMG_MODULE_HEADER* XFile::Header()
{
	return header;
}

#include "SectionNames.h"

#define min(x, y) (((x)<(y))?(x):(y))
IMG_SECTION_HEADER* XFile::GetSectionHeaderByName(const char* section_name)
{
	if(!memcmp(IMPORT_SECTION_NAME, section_name, min(sizeof(IMPORT_SECTION_NAME)-1, strlen(section_name)) ))
	{
		return &header->imports_section;
	}
	else if(!memcmp(DATA_SECTION_NAME, section_name, min(sizeof(DATA_SECTION_NAME)-1, strlen(section_name)) ))
	{
		return &header->data_section;
	}
	else if(!memcmp(CODE_SECTION_NAME, section_name, min(sizeof(CODE_SECTION_NAME)-1, strlen(section_name)) ))
	{
		return &header->code_section;
	}
	else if(!memcmp(EXPORT_SECTION_NAME, section_name, min(sizeof(EXPORT_SECTION_NAME)-1, strlen(section_name)) ))
	{
		return &header->exports_section;
	}
	else if(!memcmp(RELOC_SECTION_NAME, section_name, min(sizeof(RELOC_SECTION_NAME)-1, strlen(section_name)) ))
	{
		return &header->relocs_section;
	}
	return 0;
}

byte* XFile::GetSectionByName(const char* section_name)
{
	IMG_SECTION_HEADER* section_header = GetSectionHeaderByName(section_name);
	if(section_header)
	{
		return file + section_header->offset;
	}
	return 0;
}

dword XFile::CurrentOffset()
{
	return write_pointer;
}
