#ifdef _DEBUG
#error "This project cant be compiled in debug mode"
#endif

#include <windows.h>

int __stdcall main(unsigned int, unsigned int, unsigned int, unsigned int)
{
	MessageBox(0, "Windows", "Hello", 0);

	ExitProcess(0);
	return 0;
}