#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <shellapi.h>
#include "InjectDll.h"
#include <Dbghelp.h>

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

const IMAGE_NT_HEADERS* GetNtHeader(const BYTE* image, const DWORD imageSize);
BOOL ReadFileData(WCHAR* filename, BYTE** buff, DWORD* size);
int GetCurrentProcessBits();
int GetProcessBits(HANDLE);
void ExecuteNative64(void* shellcode, void* arg);

typedef LONG(NTAPI* RTLCREATEUSERTHREAD)(HANDLE, PSECURITY_DESCRIPTOR, BOOLEAN, ULONG, SIZE_T, SIZE_T, PTHREAD_START_ROUTINE, PVOID, PHANDLE, LPVOID);
typedef DWORD(WINAPI* GETTHREADID)(HANDLE);



BOOL LoadImports(HANDLE hProcess, BYTE* image, DWORD imageSize) {
    const IMAGE_NT_HEADERS* ntHeader = ImageNtHeader(image);
    if (ntHeader == NULL) {
        return FALSE;
    }

    // Получаем указатель на таблицу импорта.
    const IMAGE_IMPORT_DESCRIPTOR* importDesc = (const IMAGE_IMPORT_DESCRIPTOR*)ImageDirectoryEntryToData(image, TRUE, IMAGE_DIRECTORY_ENTRY_IMPORT, &imageSize);
    if (importDesc == NULL) {
        return TRUE;
    }

    // Проходим по списку библиотек, которые необходимо загрузить.
    for (; importDesc->Name != 0; importDesc++) {
        // Получаем имя библиотеки.
        const char* name = (const char*)((const BYTE*)image + importDesc->Name);
        // Загружаем библиотеку.
        HMODULE hModule = LoadLibraryA(name);
        if (hModule == NULL) {
            return FALSE;
        }

        // Получаем указатель на таблицу функций для текущей библиотеки.
        const IMAGE_THUNK_DATA* origThunk = (const IMAGE_THUNK_DATA*)((const BYTE*)image + importDesc->OriginalFirstThunk);
        if (origThunk == NULL) {
            return FALSE;
        }

        // Получаем указатель на таблицу функций для текущей библиотеки.
        IMAGE_THUNK_DATA* thunk = const_cast<IMAGE_THUNK_DATA*>(reinterpret_cast<const IMAGE_THUNK_DATA*>((const BYTE*)image + importDesc->FirstThunk));
        //const IMAGE_THUNK_DATA* thunk = (const IMAGE_THUNK_DATA*)((const BYTE*)image + importDesc->FirstThunk);
        if (thunk == NULL) {
            return FALSE;
        }

        // Проходим по списку функций и загружаем их.
        for (; origThunk->u1.AddressOfData != 0; origThunk++, thunk++) {
            DWORD_PTR funcAddress = 0;
            if (IMAGE_SNAP_BY_ORDINAL(origThunk->u1.Ordinal)) {
                // Если функция загружается по ординалу.
                funcAddress = (DWORD_PTR)GetProcAddress(hModule, (LPCSTR)IMAGE_ORDINAL(origThunk->u1.Ordinal));
            }
            else {
                // Если функция загружается по имени.
                const IMAGE_IMPORT_BY_NAME* importByName = (const IMAGE_IMPORT_BY_NAME*)((const BYTE*)image + origThunk->u1.AddressOfData);
                if (importByName == NULL) {
                    return FALSE;
                }

                funcAddress = (DWORD_PTR)GetProcAddress(hModule, (LPCSTR)importByName->Name);
            }

            if (funcAddress == 0) {
                return FALSE;
            }

            // Записываем адрес функции в таблицу импорта.
            thunk->u1.Function = funcAddress;
        }
    }

    return TRUE;
}