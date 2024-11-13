#include <stdio.h>
#include <Windows.h>

/* 실행루틴은 writefile 5바이트를 jmp mywritefile로 바꿈 -> wrietfile 함수 실행시 mywritefile로 이동함
2. 문자가 들어있는 인자를 알고리즘으로 대문자로 바꿈
3.그 다음 myfunc함수가 실행되서 원래 writefile 5바이트를 실행 한후 writefile +5로 점프 그 후 정상 실행*/

PBYTE orgfun1 = nullptr;
BYTE pOrgBytes[5]{ 0 };
DWORD dwOldProtect;
FILE* pFile = nullptr;

typedef BOOL(__fastcall* MyFunctionType)(HANDLE hFile, LPCVOID lpBuffer, DWORD nNumberOfBytesToWrite, LPDWORD lpNumberOfBytesWritten, LPOVERLAPPED lpOverlapped); // 함수 포인터 선언.

MyFunctionType myfunc = nullptr;

BOOL MyWriteFile(HANDLE hFile, LPCVOID lpBuffer, DWORD nNumberOfBytesToWrite, LPDWORD lpNumberOfBytesWritten, LPOVERLAPPED lpOverlapped)
{

	// lpBuffer에 들어있는 문자를 소문자에서 대문자로 바꿔주는 작업이다. 
	PBYTE lpBuffera = (PBYTE)malloc(nNumberOfBytesToWrite + 1);

	memcpy(lpBuffera, lpBuffer, nNumberOfBytesToWrite + 1);

	for (int i = 0; i < nNumberOfBytesToWrite; i++)
	{
		if (0x61 <= lpBuffera[i] && lpBuffera[i] <= 0x7A)
			lpBuffera[i] -= 0x20;
	}


	myfunc(hFile, lpBuffera, nNumberOfBytesToWrite, lpNumberOfBytesWritten, lpOverlapped); // 고다음 myfunc 실행 -> writefile 첨 5바이트를 실행후 writefile + 5에 있는 부분을 실행함.

	return 1;
}



PBYTE tramphook(PBYTE src, int len)
{
	BYTE pBuf1[2] = { 0x48, 0xb8 }; // mov rax, imm64
	BYTE pBuf2[2] = { 0xff, 0xe0 }; // jmp rax;

	PBYTE gateway = (PBYTE)VirtualAlloc(0, len + 12, MEM_COMMIT | MEM_RESERVE, PAGE_EXECUTE_READWRITE); // 읽기 쓰기 실행 권한을 가진 gateway 17바이트 만큼 동적할당함 

	memset(gateway, 0x90, len + 12); //5바이트 + 12바이트를 할당

	memcpy(gateway, pOrgBytes, 5); // writefile함수의  원래 5바이트를  옮김
	memcpy(&gateway[5], pBuf1, 2); // mov rax, addr;
	memcpy(&gateway[7], &src, 8); //src는 writefile +5를 가리킴 고로 우리는 src를 역참조할게 아니여서 포인터 src의 주소를 넘겨줌;
	memcpy(&gateway[15], pBuf2, 2); //jmp rax;



	return gateway; 
}



BOOL Thread()
{



	PBYTE WriteFile = (PBYTE)GetProcAddress(GetModuleHandle("kernelbase.dll"), "WriteFile"); // kernelbase의 주소에 있는 WriteFile 함수의 주소를 가져옴. 


	VirtualProtect((LPVOID)WriteFile, 5, PAGE_EXECUTE_READWRITE, &dwOldProtect);  //writefile 시작 부분의 5바이트의 권한을 변경함.

	memcpy(pOrgBytes, WriteFile, 5); //시작 5바이트만큼 옮겨줌 -> 나중에 쓸 예정
	uintptr_t dwaddress = (uintptr_t)MyWriteFile - (uintptr_t)WriteFile - 5; // MyWriteFile의 상대 주소를 구함.
	WriteFile[0] = 0xE9; // 0xe9는 jmp 상대 주소로 점프
	memcpy(&WriteFile[1], &dwaddress, 4); // 그러면 결국 jmp dwaddress가 됨.


	myfunc = (MyFunctionType)tramphook(WriteFile + 5, 5); // tramphook 함수의 결과를 리턴 == gateway의 주소



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
