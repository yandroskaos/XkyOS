#ifndef __FLOPPY_H__
#define __FLOPPY_H__

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <string>
using namespace std;

#define FLOPPY_SIZE	1474560

class FloppyWriter
{
private:
	HANDLE file;

public:
	FloppyWriter();
	~FloppyWriter();
	bool Write(BYTE* data, DWORD size);
};

#endif

