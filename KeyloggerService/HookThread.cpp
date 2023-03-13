#include "HookThread.h"
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


HookThread::HookThread(Poco::Util::Application* _app, IKeyResover* resolver, IWordProcessor* processor) : app(_app) {
    logger = &app->logger().get("AppLogger");
    keyResolver = resolver;
    wordProcessor = processor;
    pBuf = NULL;
}

void HookThread::HookProc(int nCode, WPARAM wParam, LPARAM lParam) {
    /*llr->debug("Hello WorkerThread::HookProc ");*/

    KBDLLHOOKSTRUCT* pKeyboardData = (KBDLLHOOKSTRUCT*)lParam;
    //Получим результат нажатой клавиши в символьном ввиде
    std::wstring result = keyResolver->resolve(nCode, wParam, lParam);

    if (wordProcessor == NULL) throw std::runtime_error("WordProcessor is NULL");

    HWND window = GetForegroundWindow();

    wordProcessor->process(app, window, result);
}

LRESULT HookThread::WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    Rpc* pData;
    //COPYDATASTRUCT* pcds;
    switch (message)
    {
    case WM_CREATE:
    {
        // Изменение фильтра сообщения
        BOOL result = ChangeWindowMessageFilterEx(hWnd, WM_MYMESSAGE, MSGFLT_ALLOW, NULL);
        if (!result) {
            DWORD error = GetLastError();
            // Обработка ошибки
        }
        return 0;
    }
    case WM_MYMESSAGE:
        // Извлекаем структуру COPYDATASTRUCT из параметра lParam
       // pcds = (COPYDATASTRUCT*)lParam;
        if (!pBuf)break;
        pData = (Rpc*)pBuf;
        switch (pData->getFunction()) {
        case Rpc::HookProc:
            HookProc(pData->getNCode(), pData->getWParam(), pData->getLParam());
            //if(data)delete data;
        }
        // Если размер данных соответствует ожидаемому размеру, можно привести указатель на блок памяти к типу MyData
        //if (pcds->cbData == sizeof(Rpc)) {
        //    Rpc* pData = (Rpc*)pcds->lpData;

        //    switch (pData->getFunction()) {
        //    case Rpc::HookProc:
        //        HookProc(pData->getNCode(), pData->getWParam(), pData->getLParam());
        //        //if(data)delete data;
        //    }

        //    // Важно: память, выделенную с помощью функции GlobalAlloc, необходимо освободить
        //    GlobalFree(pcds->lpData);
        //}
        //data = (Poco::SharedPtr<Rpc>*)lParam;
        //switch (data->get()->getFunction()) {
        //case Rpc::HookProc:
        //    HookProc(data->get()->getNCode(), data->get()->getWParam(), data->get()->getLParam());
        //    //if(data)delete data;
        //}
        break;
    default:
        return DefWindowProc(hWnd, message, wParam, lParam);
    }
    return 0;
}

void HookThread::run()
{
    
    HWND hDesktopWindow = GetDesktopWindow();
    DWORD dwProcessId = 0;
    GetWindowThreadProcessId(hDesktopWindow, &dwProcessId);

    //return;

    AdapterWorker<HookThread> adapter(this, &HookThread::WndProc);

    DWORD parentProcessId = GetProcessIdByName(L"explorer");

    HMODULE hModule = GetModuleHandle(NULL);
    WNDCLASS wc = { 0 };
    wc.lpfnWndProc = AdapterWorker<HookThread>::WndProc;
    wc.hInstance = hModule;
    wc.lpszClassName = L"MyClass";
    RegisterClass(&wc);

    // создание окна
    HWND hWnd = CreateWindow(L"MyClass", KEYLOGGERWINDOW, WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT,
        NULL, NULL, hModule, NULL);
    ShowWindow(hWnd, SW_SHOWNORMAL);
    UpdateWindow(hWnd);

    std::string appPath = Poco::Path(app->commandPath()).parent().toString();
    std::wstring wstr;
    Poco::UnicodeConverter::toUTF16(appPath, wstr);
    std::wstring resultStr = wstr + L"HookDll.dll";

    // Получить дескриптор главного окна Explorer
    HWND explorerHwnd = FindWindow(_T("Progman"), _T("Program Manager"));
    if (explorerHwnd == NULL)
        explorerHwnd = FindWindow(_T("Shell_TrayWnd"), NULL);


    bool result = injectDll(parentProcessId, (WCHAR*)resultStr.c_str());
    if (result) {
        logger->error("Cann't inject our Dll!");
        return;
    }

    MSG msg = { };
    while (GetMessage(&msg, NULL, 0, 0) > 0) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }
}


