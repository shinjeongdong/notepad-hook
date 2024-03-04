#include <stdio.h>
#include <Windows.h>



PBYTE orgfun1 = nullptr;
BYTE pOrgBytes[5]{ 0 };
DWORD dwOldProtect;
FILE* pFile = nullptr;

typedef BOOL(__fastcall* MyFunctionType)(HANDLE hFile, LPCVOID lpBuffer, DWORD nNumberOfBytesToWrite, LPDWORD lpNumberOfBytesWritten, LPOVERLAPPED lpOverlapped);

MyFunctionType myfunc = nullptr;

BOOL MyWriteFile(HANDLE hFile, LPCVOID lpBuffer, DWORD nNumberOfBytesToWrite, LPDWORD lpNumberOfBytesWritten, LPOVERLAPPED lpOverlapped)
{





	PBYTE lpBuffera = (PBYTE)malloc(nNumberOfBytesToWrite + 1);

	memcpy(lpBuffera, lpBuffer, nNumberOfBytesToWrite + 1);

	for (int i = 0; i < nNumberOfBytesToWrite; i++)
	{
		if (0x61 <= lpBuffera[i] && lpBuffera[i] <= 0x7A)
			lpBuffera[i] -= 0x20;
	}


	myfunc(hFile, lpBuffera, nNumberOfBytesToWrite, lpNumberOfBytesWritten, lpOverlapped);

	return 1;
}



PBYTE tramphook(PBYTE src, PBYTE dst, int len)
{
	BYTE pBuf1[2] = { 0x48, 0xb8};
	BYTE pBuf2[2] = { 0xff, 0xe0 };
	
	PBYTE gateway = (PBYTE)VirtualAlloc(0, len + 12, MEM_COMMIT | MEM_RESERVE, PAGE_EXECUTE_READWRITE);

	memset(gateway, 0x90, len + 12);

	memcpy(gateway, pOrgBytes, 5);
	memcpy(&gateway[5], pBuf1, 2);
	memcpy(&gateway[7], &src, 8);
	memcpy(&gateway[15], pBuf2, 2);

	
	
	return gateway;
}



BOOL Thread()
{
	

	
	PBYTE WriteFile = (PBYTE)GetProcAddress(GetModuleHandle("kernelbase.dll"), "WriteFile");
	

	VirtualProtect((LPVOID)WriteFile, 5, PAGE_EXECUTE_READWRITE, &dwOldProtect);

	memcpy(pOrgBytes, WriteFile, 5);
	uintptr_t dwaddress = (uintptr_t)MyWriteFile - (uintptr_t)WriteFile - 5;
	WriteFile[0] = 0xE9;
	memcpy(&WriteFile[1], &dwaddress, 4);

	
	myfunc = (MyFunctionType)tramphook(WriteFile + 5, (PBYTE)MyWriteFile, 5);



	return 1;
}


BOOL WINAPI DllMain(
	HINSTANCE hinstDLL, // handle to DLL module
	DWORD fdwReason, // reason for calling function
	LPVOID lpvReserved) // reserved
{

	switch (fdwReason)
	{
	case DLL_PROCESS_ATTACH:
		CreateThread(0, 0, (LPTHREAD_START_ROUTINE)Thread, hinstDLL, 0, 0);

		break;

	case DLL_THREAD_ATTACH:

		break;

	case DLL_THREAD_DETACH:

		break;

	case DLL_PROCESS_DETACH:

		if (lpvReserved != nullptr)
		{
			break;
		}

		break;
	}
	return TRUE;
}

