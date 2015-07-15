#ifndef __WINDOWS_H__
#define __WINDOWS_H__

typedef dword HANDLE;

//==================================CODE======================================//
#pragma code_seg(".code")
//============================================================================//

PUBLIC void CloseHandle(HANDLE handle)
{
	HEAP_Free((VIRTUAL&)handle);
}

#endif
