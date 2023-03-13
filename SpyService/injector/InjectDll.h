#pragma once

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <Windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <shellapi.h>
#include "InjectDll.h"

#if defined(_M_IX86)
# if defined(_DEBUG)
// debug code is broken
//#  include "InjectWow64toNative64d.h"
#  include "InjectWow64toNative64.h"
# else
#  include "InjectWow64toNative64.h"
# endif
#endif

// Loader shellcode from https ://github.com/UserExistsError/DllLoaderShellcode
#include "loaders.h"
#include <iostream>

#pragma pack(push, 1)
struct InjectArgs {
	UINT64 start; // remote shellcode address
	UINT64 hProcess; // handle of process to inject
	UINT64 hThread; // new thread id
};
#pragma pack(pop)

const IMAGE_NT_HEADERS* GetNtHeader(const BYTE* image, const DWORD imageSize);
BOOL ReadFileData(WCHAR* filename, BYTE** buff, DWORD* size);
int GetCurrentProcessBits();
int GetProcessBits(HANDLE);
void ExecuteNative64(void* shellcode, void* arg);

typedef LONG(NTAPI* RTLCREATEUSERTHREAD)(HANDLE, PSECURITY_DESCRIPTOR, BOOLEAN, ULONG, SIZE_T, SIZE_T, PTHREAD_START_ROUTINE, PVOID, PHANDLE, LPVOID);
typedef DWORD(WINAPI* GETTHREADID)(HANDLE);

DWORD GetThreadIdFromHandle(HANDLE hThread);

BOOL wcs2dw(WCHAR* wp, DWORD* dp);

int injectDll(DWORD processId, WCHAR* inctingDll);

