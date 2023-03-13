#include "WorkerThread.h"
#include <Poco/UnicodeConverter.h>
#include <tchar.h>
#include <format>

using namespace std;

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


WorkerThread::WorkerThread(Poco::Util::Application* _app) : app(_app) {
    logger = &app->logger().get("AppLogger");
    pBuf = NULL;
}

void WorkerThread::run()
{

    //HWND hDesktopWindow = GetDesktopWindow();
    //DWORD dwProcessId = 0;
    //GetWindowThreadProcessId(hDesktopWindow, &dwProcessId);


    DWORD parentProcessId = GetProcessIdByName(L"explorer");


    std::string appPath = Poco::Path(app->commandPath()).parent().toString();
    std::wstring wstr;
    Poco::UnicodeConverter::toUTF16(appPath, wstr);
    std::wstring resultStr = wstr + L"ServiceDll.dll";

    // Получить дескриптор главного окна Explorer
 /*   HWND explorerHwnd = FindWindow(_T("Progman"), _T("Program Manager"));
    if (explorerHwnd == NULL)
        explorerHwnd = FindWindow(_T("Shell_TrayWnd"), NULL);*/

    bool result = injectDll(parentProcessId, (WCHAR*)resultStr.c_str());
    if (result) {
        logger->error("Cann't inject our Dll!");
        return;
    }

}


