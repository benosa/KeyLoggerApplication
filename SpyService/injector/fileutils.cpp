#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include <stdio.h>
#include <iostream>

const IMAGE_NT_HEADERS* GetNtHeader(const BYTE* image, const DWORD imageSize)
{
	const IMAGE_DOS_HEADER* dosHeader = (IMAGE_DOS_HEADER*)image;
	if (dosHeader->e_magic != IMAGE_DOS_SIGNATURE) {
		wprintf(L"Invalid DOS header\n");
		return NULL;
	}
	const IMAGE_NT_HEADERS* ntHeader = (IMAGE_NT_HEADERS*)((ULONG_PTR)image + dosHeader->e_lfanew);
	if ((BYTE*)ntHeader < image) {
		wprintf(L"Invalid NT header\n");
		return NULL;
	}
	if ((BYTE*)ntHeader > (image + imageSize - sizeof(IMAGE_NT_HEADERS))) {
		wprintf(L"Invalid NT header\n");
		return NULL;
	}
	if (ntHeader->Signature != IMAGE_NT_SIGNATURE) {
		wprintf(L"Invalid NT header\n");
		return NULL;
	}
	return ntHeader;
}
