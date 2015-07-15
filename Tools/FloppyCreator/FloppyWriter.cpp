#include "FloppyWriter.h"

#define DRIVE_A "\\\\.\\A:"

FloppyWriter::FloppyWriter()
{
	file=CreateFile(DRIVE_A, GENERIC_WRITE, FILE_SHARE_READ, 0, OPEN_EXISTING, 0, 0);

	if(file==INVALID_HANDLE_VALUE)
		throw ::GetLastError();
}

FloppyWriter::~FloppyWriter()
{
	CloseHandle(file);
}

bool FloppyWriter::Write(BYTE* data, DWORD size)
{
	DWORD bytes_read;
	return WriteFile(file, data, size, &bytes_read, 0);
}

