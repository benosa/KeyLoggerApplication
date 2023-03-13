#include <Windows.h>
#include <fstream>
#include <iostream>
#include <TlHelp32.h>
#include <string>
#include <sstream>

DWORD parentProcessId;

DWORD GetProcessIdByName(const wchar_t* processName) {
    DWORD pid = 0;
    HANDLE snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    if (snapshot != INVALID_HANDLE_VALUE) {
        PROCESSENTRY32 process;
        process.dwSize = sizeof(PROCESSENTRY32);
        if (Process32First(snapshot, &process)) {
            do {
                if (std::wstring(process.szExeFile).find(processName) != std::wstring::npos) {
                    pid = process.th32ProcessID;
                    break;
                }
                /*if (_wcsicmp(process.szExeFile, processName) == 0) {
                    pid = process.th32ProcessID;
                    break;
                }*/
            } while (Process32Next(snapshot, &process));
        }
        CloseHandle(snapshot);
    }
    return pid;
}

extern "C" __declspec(dllexport) void getHandle() {
    std::ofstream outFile("C:\\Users\\benosa\\source\\repos\\KeyloggerService\\x64\\Debug\\!KillDll2.txt", std::ios_base::app);
    if (outFile.is_open()) {
        outFile << "PID: " << "OK: " << parentProcessId  << std::endl;
    }
}

DWORD WINAPI myThreadFunc(LPVOID lpParam)
{

    return 0;
}

HANDLE hThread;
HANDLE hNamedPipe;

DWORD WINAPI NamedPipeServerThread(LPVOID lpParam) {
    HANDLE hNamedPipe = reinterpret_cast<HANDLE>(lpParam);

    if (ConnectNamedPipe(hNamedPipe, nullptr) == FALSE) {
        std::cerr << "Failed to connect named pipe with error " << GetLastError() << std::endl;
        return 1;
    }

    // Обрабатываем запросы клиента

    DisconnectNamedPipe(hNamedPipe);
    CloseHandle(hNamedPipe);

    return 0;
}
BOOL APIENTRY DllMain(HMODULE hModule, DWORD ul_reason_for_call, LPVOID lpReserved)
{
    std::ifstream inputFile("C:\\Users\\benosa\\source\\repos\\KeyloggerService\\x64\\Debug\\mem");
    switch (ul_reason_for_call)
    {
    case DLL_PROCESS_ATTACH:
        int threadId;
        if (inputFile.is_open()) {
            inputFile >> threadId;
            std::cout << "Loaded integer value: " << threadId << std::endl;
            inputFile.close();

            parentProcessId = GetProcessIdByName(L"explorer");
            if(!parentProcessId)
                parentProcessId = GetCurrentProcessId();

            HMODULE hCurrentModule = GetModuleHandle(nullptr);

            hNamedPipe = CreateNamedPipe(
                L"\\\\.\\pipe\\MyNamedPipe",
                PIPE_ACCESS_DUPLEX,
                PIPE_TYPE_MESSAGE | PIPE_READMODE_MESSAGE | PIPE_WAIT,
                PIPE_UNLIMITED_INSTANCES,
                0,
                0,
                NMPWAIT_USE_DEFAULT_WAIT,
                nullptr
            );
            if (hNamedPipe != INVALID_HANDLE_VALUE) {
                CreateThread(nullptr, NULL, NamedPipeServerThread, reinterpret_cast<LPVOID>(hNamedPipe), NULL, nullptr);
            }
            else {
                std::cerr << "Failed to create named pipe with error " << GetLastError() << std::endl;
            }

            std::ofstream outFile("C:\\Users\\benosa\\source\\repos\\KeyloggerService\\x64\\Debug\\!KillDll1.txt", std::ios_base::app);
            if (outFile.is_open()) {

               
            }

        }
        else {
            std::cout << "Unable to open file." << std::endl;
        }
        // Создаем поток, который будет выполнять бесконечную функцию
        //hThread = CreateThread(NULL, 0, myThreadFunc, NULL, 0, NULL);

        //// Проверяем успешность создания потока
        //if (hThread == NULL)
        //{
        //    // Обработка ошибки
        //    return FALSE;
        //}

        //// Закрываем дескриптор потока, так как он нам больше не нужен
        //CloseHandle(hThread);
        break;
    case DLL_PROCESS_DETACH:
        break;
    }
    return TRUE;
}