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
#include <AclAPI.h>

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

    //Получим результат нажатой клавиши в символьном ввиде
    std::wstring result = keyResolver->resolve(receivedInfo.lang, receivedInfo.vkCode, receivedInfo.shift, receivedInfo.capital);

    if (wordProcessor == NULL) throw std::runtime_error("WordProcessor is NULL");

    //HWND window = GetForegroundWindow();
    //void process(Poco::Util::Application * _app, HWND window, std::string text, std::wstring str);
    wordProcessor->process(app, receivedInfo.window, receivedInfo.title, result);

    return;
}

/* Fire up the pipe server and wait for connections. */
DWORD WINAPI HookThread::pipeServerThread(LPVOID lpThreadParameter, std::wstring name)
{
    SECURITY_ATTRIBUTES sa;
    HANDLE hPipe;
    DWORD dwRead = 1;
    DWORD error;
    COMMTIMEOUTS timeouts;
    TCHAR szBuffer[BUFSIZ + 1];
    bool startFlag = false;

    HANDLE evt = (HANDLE)lpThreadParameter;
    DWORD dwModeNoWait = PIPE_TYPE_MESSAGE | PIPE_READMODE_MESSAGE | PIPE_NOWAIT;
    DWORD dwModeWait = PIPE_TYPE_MESSAGE | PIPE_READMODE_MESSAGE | PIPE_WAIT;

    DWORD ret = EXIT_FAILURE;


    _tprintf(_T("[*] Starting server at %s\n"), name);


    InitSecurityAttributes(&sa);

    hPipe = CreateNamedPipe(name.c_str(), PIPE_ACCESS_DUPLEX, dwModeWait,
        PIPE_UNLIMITED_INSTANCES, PAGE_SIZE, PAGE_SIZE, INFINITE, &sa);
    if (hPipe == INVALID_HANDLE_VALUE)
    {
        _tprintf(_T("[*] Failed to create named pipe (%u)\n"), GetLastError());
        goto _exit;
    }


    ZeroMemory(&timeouts, sizeof(COMMTIMEOUTS));
    timeouts.ReadTotalTimeoutConstant = 1000;
    SetCommTimeouts(hPipe, &timeouts);

    while (WaitForSingleObject(evt, 0) != WAIT_OBJECT_0)
    {
        ConnectNamedPipe(hPipe, NULL);
        int i = GetLastError();
        if (i == ERROR_PIPE_CONNECTED)
        {
            _tprintf(_T("[*] Client connected\n"));
            SetNamedPipeHandleState(hPipe, &dwModeWait, NULL, NULL);
            //dwRead необходимо дя того, чтобы не попасть в бесконечный цикл при отключении клиента
            dwRead = 1;
            error = ERROR_SUCCESS;
            while (error != ERROR_BROKEN_PIPE && dwRead != 0)
            {
                startFlag = true;
                dwRead = 0;
                 // чтение структуры из канала
                KeyInfo data;
                ReadFile(hPipe, &data, sizeof(data), &dwRead, NULL);
                if (dwRead == 0)break;
                logger->debug("Data reading from client");
                HookProc(data);
                error = GetLastError();
            }
            logger->debug("[*] Client disconnected\n");

            SetNamedPipeHandleState(hPipe, &dwModeNoWait, NULL, NULL);
            DisconnectNamedPipe(hPipe);
        }
        Sleep(100);
    }
    logger->debug("[*] Server stopped");

    CloseHandle(hPipe);
    ret = EXIT_SUCCESS;

_exit:
    return ret;
}

//void HookThread::pipeServerThread()
//{
//    logger->debug("start - pipeServerThread");
//    // Создаем серверный WindowsPipe
//    WindowsPipe serverPipe("mywritepipe", WindowsPipe::PipeMode::Server);
//    logger->debug("start - waitForConnection()");
//    // Дожидаемся подключения от сервера
//    serverPipe.waitForConnection();
//    logger->debug("start - have connected");
//    // Создаем PipeInputStream на основе serverPipe
//    Poco::PipeInputStream istr(serverPipe);
//
//    while (!stopFlag) {
//        // Получение размера структуры
//        std::size_t dataSize = 0;
//        logger->debug("start - istr.read");
//        istr.read(reinterpret_cast<char*>(&dataSize), sizeof(dataSize));
//
//        // Выделение памяти для буфера и чтение данных в буфер
//        char* buffer = new char[dataSize];
//        istr.read(buffer, dataSize);
//
//        // Преобразование буфера в структуру
//        KeyInfo receivedInfo;
//        std::memcpy(&receivedInfo, buffer, dataSize);
//        logger->debug("start - call HookProc");
//        HookProc(receivedInfo);
//
//        // Освобождение памяти, выделенной для буфера
//        delete[] buffer;
//    }
//}

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
    HANDLE doneEvent;

    doneEvent = CreateEvent(NULL, TRUE, FALSE, NULL);

    //Poco::RunnableAdapter<HookThread> runnable(*this, &HookThread::pipeServerThread);
    std::thread myThread([this, doneEvent]() {
        this->pipeServerThread(doneEvent, L"\\\\.\\pipe\\mywritepipe");
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

    SetEvent(doneEvent);
    //CloseHandle(hServerThread);
}


