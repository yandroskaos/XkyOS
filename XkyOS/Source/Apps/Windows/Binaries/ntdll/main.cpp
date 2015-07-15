#ifdef _DEBUG
#error "This project cant be compiled in debug mode"
#endif

#define WINDOWS_API_SERVICES	0x40

#define CALL0(X)	__asm mov eax, X \
					__asm int WINDOWS_API_SERVICES \
					__asm ret 

#define CALL1(X)	__asm push ebp \
					__asm mov ebp, esp \
					__asm push dword ptr [ebp + 8] \
					__asm mov eax, X \
					__asm int WINDOWS_API_SERVICES \
					__asm add esp, 4 \
					__asm pop ebp \
					__asm ret 4

#define CALL2(X)	__asm push ebp \
					__asm mov ebp, esp \
					__asm push dword ptr [ebp + 12] \
					__asm push dword ptr [ebp + 8] \
					__asm mov eax, X \
					__asm int WINDOWS_API_SERVICES \
					__asm add esp, 8 \
					__asm pop ebp \
					__asm ret 8

#define CALL3(X)	__asm push ebp \
					__asm mov ebp, esp \
					__asm push dword ptr [ebp + 16] \
					__asm push dword ptr [ebp + 12] \
					__asm push dword ptr [ebp + 8] \
					__asm mov eax, X \
					__asm int WINDOWS_API_SERVICES \
					__asm add esp, 12 \
					__asm pop ebp \
					__asm ret 12

#define CALL4(X)	__asm push ebp \
					__asm mov ebp, esp \
					__asm push dword ptr [ebp + 20] \
					__asm push dword ptr [ebp + 16] \
					__asm push dword ptr [ebp + 12] \
					__asm push dword ptr [ebp + 8] \
					__asm mov eax, X \
					__asm int WINDOWS_API_SERVICES \
					__asm add esp, 16 \
					__asm pop ebp \
					__asm ret 16

#define CALL5(X)	__asm push ebp \
					__asm mov ebp, esp \
					__asm push dword ptr [ebp + 24] \
					__asm push dword ptr [ebp + 20] \
					__asm push dword ptr [ebp + 16] \
					__asm push dword ptr [ebp + 12] \
					__asm push dword ptr [ebp + 8] \
					__asm mov eax, X \
					__asm int WINDOWS_API_SERVICES \
					__asm add esp, 20 \
					__asm pop ebp \
					__asm ret 20

#define WINDOWS_SERVICE_MessageBoxWaiting	1
#define WINDOWS_SERVICE_MessageBox			2
#define WINDOWS_SERVICE_ExitProcess			3

__declspec(naked) void __stdcall InternalMessageBox(unsigned int, const char* _message, const char* _title, unsigned int)
{
	__asm
	{
		push ebp
		mov ebp, esp
		//Call MessageBox
		push dword ptr [ebp + 20]
		push dword ptr [ebp + 16]
		push dword ptr [ebp + 12]
		push dword ptr [ebp + 8]
		mov eax, WINDOWS_SERVICE_MessageBox
		int WINDOWS_API_SERVICES
		add esp, 16

		//Call MessageBoxWaiting
_WaitingForClick:
		mov eax, WINDOWS_SERVICE_MessageBoxWaiting
		int WINDOWS_API_SERVICES
		test eax, eax
		jz _WaitingForClick

		pop ebp
		ret 16
	}
}

__declspec(naked) void  __stdcall InternalExitProcess(unsigned int _exit_code)
{
	CALL1(WINDOWS_SERVICE_ExitProcess)
}
