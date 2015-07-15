#ifndef __PE_H__
#define __PE_H__

//==================================DATA======================================//
#pragma data_seg(".data")
//============================================================================//

struct PE_IMAGE_DOS_HEADER
{
	word   e_magic;
	word   e_cblp;
	word   e_cp;
	word   e_crlc;
	word   e_cparhdr;
	word   e_minalloc;
	word   e_maxalloc;
	word   e_ss;
	word   e_sp;
	word   e_csum;
	word   e_ip;
	word   e_cs;
	word   e_lfarlc;
	word   e_ovno;
	word   e_res[4];
	word   e_oemid;
	word   e_oeminfo;
	word   e_res2[10];
	dword  e_lfanew;
};

struct PE_IMAGE_FILE_HEADER
{
	word    Machine;
	word    NumberOfSections;
	dword   TimeDateStamp;
	dword   PointerToSymbolTable;
	dword   NumberOfSymbols;
	word    SizeOfOptionalHeader;
	word    Characteristics;
};

struct PE_IMAGE_DATA_DIRECTORY
{
	dword   VirtualAddress;
	dword   Size;
};

#define PE_IMAGE_NUMBEROF_DIRECTORY_ENTRIES 16

struct PE_IMAGE_OPTIONAL_HEADER
{
	word    Magic;
	byte    MajorLinkerVersion;
	byte    MinorLinkerVersion;
	dword   SizeOfCode;
	dword   SizeOfInitializedData;
	dword   SizeOfUninitializedData;
	dword   AddressOfEntryPoint;
	dword   BaseOfCode;
	dword   BaseOfData;
	dword   ImageBase;
	dword   SectionAlignment;
	dword   FileAlignment;
	word    MajorOperatingSystemVersion;
	word    MinorOperatingSystemVersion;
	word    MajorImageVersion;
	word    MinorImageVersion;
	word    MajorSubsystemVersion;
	word    MinorSubsystemVersion;
	dword   Win32VersionValue;
	dword   SizeOfImage;
	dword   SizeOfHeaders;
	dword   CheckSum;
	word    Subsystem;
	word    DllCharacteristics;
	dword   SizeOfStackReserve;
	dword   SizeOfStackCommit;
	dword   SizeOfHeapReserve;
	dword   SizeOfHeapCommit;
	dword   LoaderFlags;
	dword   NumberOfRvaAndSizes;
	PE_IMAGE_DATA_DIRECTORY DataDirectory[PE_IMAGE_NUMBEROF_DIRECTORY_ENTRIES];
};

struct PE_IMAGE_NT_HEADERS
{
	dword Signature;
	PE_IMAGE_FILE_HEADER FileHeader;
	PE_IMAGE_OPTIONAL_HEADER OptionalHeader;
};


//==================================CODE======================================//
#pragma code_seg(".code")
//============================================================================//

#endif
