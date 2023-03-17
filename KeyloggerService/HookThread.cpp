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

extern std::atomic<bool> done;
extern std::atomic<bool> done_callback;

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


HookThread::HookThread(HANDLE done, Poco::Util::Application* _app, IKeyResover* resolver, IWordProcessor* processor) : doneEvent(&done), app(_app) {
    logger = &app->logger().get("AppLogger");
    keyResolver = resolver;
    wordProcessor = processor;
    pBuf = NULL;
    serverPipe = NULL;
}

void HookThread::HookProc(KeyInfo receivedInfo) {

    //Получим результат нажатой клавиши в символьном ввиде
    std::wstring result = keyResolver->resolve(receivedInfo.lang, receivedInfo.vkCode, receivedInfo.shift, receivedInfo.capital);

    if (wordProcessor == NULL) throw std::runtime_error("WordProcessor is NULL");
    wordProcessor->process(app, receivedInfo.window, receivedInfo.title, result);

    return;
}

/* Fire up the pipe server and wait for connections. */
DWORD WINAPI HookThread::pipeServerThread(LPVOID lpThreadParameter, std::wstring name)
{
    SECURITY_ATTRIBUTES sa;
    serverPipe;
    DWORD dwRead = 1;
    DWORD error;
    COMMTIMEOUTS timeouts;
    TCHAR szBuffer[BUFSIZ + 1];
    bool startFlag = false;

    HANDLE evt = (HANDLE)lpThreadParameter;
    DWORD dwModeNoWait = PIPE_TYPE_MESSAGE | PIPE_READMODE_MESSAGE | PIPE_NOWAIT;
    DWORD dwModeWait = PIPE_TYPE_MESSAGE | PIPE_READMODE_MESSAGE | PIPE_WAIT;

    DWORD ret = EXIT_FAILURE;
    std::string _name;
    Poco::UnicodeConverter::toUTF8(name, _name);

    logger->debug("[*] Starting server at" + _name);

    InitSecurityAttributes(&sa);

    serverPipe = CreateNamedPipe(name.c_str(), PIPE_ACCESS_DUPLEX, dwModeWait,
        PIPE_UNLIMITED_INSTANCES, PAGE_SIZE, PAGE_SIZE, INFINITE, &sa);
    if (serverPipe == INVALID_HANDLE_VALUE)
    {
        logger->debug("[*] Failed to create named pipe (" + std::to_string(GetLastError()) + ")");
        goto _exit;
    }


    ZeroMemory(&timeouts, sizeof(COMMTIMEOUTS));
    timeouts.ReadTotalTimeoutConstant = 1000;
    SetCommTimeouts(serverPipe, &timeouts);

    while (!done)
    {
        ConnectNamedPipe(serverPipe, NULL);
        int i = GetLastError();
        if (i == ERROR_PIPE_CONNECTED)
        {
            logger->debug("[*] Client connected");
            SetNamedPipeHandleState(serverPipe, &dwModeWait, NULL, NULL);
            //dwRead = 1 необходимо дя того, чтобы не попасть в бесконечный цикл при отключении клиента
            dwRead = 1;
            error = ERROR_SUCCESS;
            while (error != ERROR_BROKEN_PIPE && dwRead != 0 && !done)
            {
                startFlag = true;
                dwRead = 0;
                 // чтение структуры из канала
                KeyInfo data;
                ReadFile(serverPipe, &data, sizeof(data), &dwRead, NULL);
                if (dwRead == 0)break;
                logger->debug("Data reading from client");
                HookProc(data);
                error = GetLastError();
            }
            logger->debug("[*] Client disconnected\n");

            SetNamedPipeHandleState(serverPipe, &dwModeNoWait, NULL, NULL);
            DisconnectNamedPipe(serverPipe);
        }
        Sleep(100);
    }
    if (logger)logger->debug("[*] Server stopped");

    CloseHandle(serverPipe);
    ret = EXIT_SUCCESS;

_exit:
    return ret;
}

void HookThread::sendCommand() {

    HANDLE writePipe = CreateFile(
        L"\\\\.\\pipe\\myreadpipe",
        GENERIC_WRITE,
        0,
        NULL,
        OPEN_EXISTING,
        0,
        NULL);

    if (writePipe == INVALID_HANDLE_VALUE) {
        logger->debug("[*] Send Command Error. Failed to open named pipe: " + GetLastError());
    }
    std::string out("stop");
    DWORD bytesWritten = 0;
    WriteFile(writePipe, out.c_str(), out.size(), &bytesWritten, NULL);
    if (bytesWritten != out.size())
    {
        logger->debug("[*] Send Command to Client Error! Failed write structure size to named pipe: " + GetLastError());
    }

    //// Создаем серверный WindowsPipe
    //WindowsPipe clientPipe("myreadpipe", WindowsPipe::PipeMode::Client);
    //// Создаем PipeInputStream на основе serverPipe
    //Poco::PipeOutputStream ostr(clientPipe);
    //ostr << "stop";
    logger->debug("[*] Send STOP Command to Client!");
    Sleep(2000);
    done_callback = true;
}

void serverThreadFunc()
{

}

void HookThread::stop() {
    // Отменить операцию чтения в канале сервера, чтобы выйти из блокировки чтения
    if (!CancelIo(serverPipe)) {
        // Обработка ошибки
    }
    sendCommand();
    //PostQuitMessage(0);
}

void HookThread::run()
{
    

    //doneEvent = CreateEvent(NULL, TRUE, FALSE, NULL);

    //Poco::RunnableAdapter<HookThread> runnable(*this, &HookThread::pipeServerThread);
    std::thread myThread([&]() {
        this->pipeServerThread(doneEvent, L"\\\\.\\pipe\\mywritepipe");
        });
    myThread.detach();

    std::this_thread::sleep_for(std::chrono::seconds(2));

    DWORD parentProcessId = GetProcessIdByName(L"explorer");

    std::string appPath = Poco::Path(app->commandPath()).parent().toString();
    std::wstring wstr;
    Poco::UnicodeConverter::toUTF16(appPath, wstr);
    std::wstring resultStr = wstr + L"HookDll.dll";

    bool result = injectDll(parentProcessId, (WCHAR*)resultStr.c_str());
    if (result) {
        logger->error("Cann't inject our Dll!");
        stop();
        return;
    }

    /*MSG msg = { };
    while (GetMessage(&msg, NULL, 0, 0) > 0 && WaitForSingleObject(doneEvent, 0) != WAIT_OBJECT_0) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }*/
    while (!done) {
        Sleep(1000);
    }
    logger->debug("[*] Send STOP Command to Client!");
    stop();
    //CloseHandle(hServerThread);
}


