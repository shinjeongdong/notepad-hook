#include <stdio.h>
#include <Windows.h>


PBYTE orgfunc = nullptr;
FARPROC orgfun1 = nullptr;
BYTE pOrgBytes[5]{ 0 };
DWORD dwOldProtect;

BOOL MyWriteFile(HANDLE hFile, LPCVOID lpBuffer, DWORD nNumberOfBytesToWrite, LPDWORD lpNumberOfBytesWritten, LPOVERLAPPED lpOverlapped)
{
  
    typedef BOOL(__fastcall* MyFunctionType)(HANDLE hFile, LPCVOID lpBuffer, DWORD nNumberOfBytesToWrite, LPDWORD lpNumberOfBytesWritten, LPOVERLAPPED lpOverlapped);

    MyFunctionType myfunc = (MyFunctionType)orgfun1;

    PBYTE lpBuffera = (PBYTE)malloc(nNumberOfBytesToWrite + 1);
    
    memcpy(lpBuffera, lpBuffer, nNumberOfBytesToWrite+1);
    for (int i = 0; i < nNumberOfBytesToWrite; i++)
    {
        if (0x61 <= lpBuffera[i] && lpBuffera[i] <= 0x7A)
            lpBuffera[i] -= 0x20;
    }

    
  
    return myfunc(hFile, lpBuffera, nNumberOfBytesToWrite, lpNumberOfBytesWritten, lpOverlapped);
}





BOOL HackThread()
{
   
    orgfunc = (PBYTE)GetProcAddress(GetModuleHandle("kernel32.dll"), "WriteFile");
    orgfun1 = GetProcAddress(GetModuleHandle("kernelbase.dll"), "WriteFile");
    

    VirtualProtect((LPVOID)orgfunc, 5, PAGE_EXECUTE_READWRITE, &dwOldProtect);

    memcpy(pOrgBytes, orgfunc, 5);

    DWORD dwAddress = (DWORD)MyWriteFile - (DWORD)orgfunc - 5;

    orgfunc[0] = 0xE9;
    memcpy(&orgfunc[1], &dwAddress, 4);

    FILE* pFile = nullptr;
    if (AllocConsole()) {
        freopen_s(&pFile, "CONIN$", "rb", stdin);
        freopen_s(&pFile, "CONOUT$", "wb", stdout);
        freopen_s(&pFile, "CONOUT$", "wb", stderr);
    }
    else
    {
        return 0;
    }
    printf("%p\n", orgfunc);

}

















BOOL WINAPI DllMain(
    HINSTANCE hinstDLL,  // handle to DLL module
    DWORD fdwReason,     // reason for calling function
    LPVOID lpvReserved)  // reserved
{
  
    switch (fdwReason)
    {
    case DLL_PROCESS_ATTACH:
        CreateThread(0, 0, (LPTHREAD_START_ROUTINE)HackThread, hinstDLL, 0, 0);
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