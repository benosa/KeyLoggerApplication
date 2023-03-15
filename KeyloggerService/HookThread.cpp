#include "HookThread.h"
#include <Poco/UnicodeConverter.h>
#include <tchar.h>
#include <format>
#include "defines.h"
#include <Poco/PipeStream.h>
#include "WindowsPipe.h"
#include <Poco/Runnable.h>
#include <Poco/RunnableAdapter.h>
#include <thread>

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

void HookThread::HookProc(KeyInfo receivedInfo) {

    KBDLLHOOKSTRUCT* pKeyboardData = (KBDLLHOOKSTRUCT*)receivedInfo.plParam;
    //Получим результат нажатой клавиши в символьном ввиде
    std::wstring result = keyResolver->resolve(receivedInfo.lang, receivedInfo.pnCode, receivedInfo.pwParam, receivedInfo.plParam);

    if (wordProcessor == NULL) throw std::runtime_error("WordProcessor is NULL");

    //HWND window = GetForegroundWindow();
    void process(Poco::Util::Application * _app, HWND window, std::string text, std::wstring str);
    wordProcessor->process(app, receivedInfo.window, receivedInfo.text, result);

    return;
}

void HookThread::pipeServerThread()
{
    logger->debug("start - pipeServerThread");
    // Создаем серверный WindowsPipe
    WindowsPipe serverPipe("mywritepipe", WindowsPipe::PipeMode::Server);
    logger->debug("start - waitForConnection()");
    // Дожидаемся подключения от сервера
    serverPipe.waitForConnection();
    logger->debug("start - have connected");
    // Создаем PipeInputStream на основе serverPipe
    Poco::PipeInputStream istr(serverPipe);

    while (!stopFlag) {
        // Получение размера структуры
        std::size_t dataSize = 0;
        logger->debug("start - istr.read");
        istr.read(reinterpret_cast<char*>(&dataSize), sizeof(dataSize));

        // Выделение памяти для буфера и чтение данных в буфер
        char* buffer = new char[dataSize];
        istr.read(buffer, dataSize);

        // Преобразование буфера в структуру
        KeyInfo receivedInfo;
        std::memcpy(&receivedInfo, buffer, dataSize);
        logger->debug("start - call HookProc");
        HookProc(receivedInfo);

        // Освобождение памяти, выделенной для буфера
        delete[] buffer;
    }
}

void HookThread::sendCommand() {
    // Создаем серверный WindowsPipe
    WindowsPipe clientPipe("myreadpipe", WindowsPipe::PipeMode::Client);
    // Создаем PipeInputStream на основе serverPipe
    Poco::PipeOutputStream ostr(clientPipe);
    ostr << "stop";
}

void serverThreadFunc()
{

}

void HookThread::stop() {
    stopFlag = true;
    sendCommand();
    PostQuitMessage(0);
}

void HookThread::run()
{
    //Poco::RunnableAdapter<HookThread> runnable(*this, &HookThread::pipeServerThread);
    std::thread myThread([this]() {
        this->pipeServerThread();
        });
    myThread.detach();

    std::this_thread::sleep_for(std::chrono::seconds(2));

    DWORD parentProcessId = GetProcessIdByName(L"explorer");

    std::string appPath = Poco::Path(app->commandPath()).parent().toString();
    std::wstring wstr;
    Poco::UnicodeConverter::toUTF16(appPath, wstr);
    std::wstring resultStr = wstr + L"HookDll.dll";

    /*bool result = injectDll(parentProcessId, (WCHAR*)resultStr.c_str());
    if (result) {
        logger->error("Cann't inject our Dll!");
        stop();
        return;
    }*/

    MSG msg = { };
    while (GetMessage(&msg, NULL, 0, 0) > 0) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }
}


