#include "CommandProcessor.h"

CommandProcessor::CommandProcessor(std::wstring pipePath, Poco::Logger* _logger, std::function<void()> _callback): logger(_logger), callback(_callback)
{
    stopFlag = false;

    hNamedPipe = CreateNamedPipe(
        pipePath.c_str(),
        PIPE_ACCESS_DUPLEX,
        PIPE_TYPE_MESSAGE | PIPE_READMODE_MESSAGE | PIPE_WAIT,
        PIPE_UNLIMITED_INSTANCES,
        1024,
        1024,
        NMPWAIT_USE_DEFAULT_WAIT,
        nullptr
    );

    if (hNamedPipe != INVALID_HANDLE_VALUE) {
        AdapterWorker<CommandProcessor, DWORD WINAPI, LPVOID> adapter(this, &CommandProcessor::NamedPipeServerThread);
        CreateThread(nullptr, NULL, AdapterWorker<CommandProcessor, DWORD WINAPI, LPVOID>::Function, hNamedPipe, NULL, nullptr);
    }
    else {
        logger->get("CommandProcessor").error("Failed to create named pipe with error " + GetLastError());
    }
}

void CommandProcessor::setStopPipeFlag() {
    stopFlag = true;
}

DWORD WINAPI CommandProcessor::NamedPipeServerThread(LPVOID lpParam) {
    HANDLE hPipe = reinterpret_cast<HANDLE>(lpParam);

    // Wait for a client to connect
    while (!stopFlag)
    {
        /*DWORD dwMode = PIPE_NOWAIT;
        if (!SetNamedPipeHandleState(hPipe, &dwMode, nullptr, nullptr))
        {
            logger.get("CommandProcessor").error("Failed to set named pipe handle state. Error code: " + GetLastError());
            return 1;
        }*/

        if (ConnectNamedPipe(hPipe, nullptr) != FALSE)
        {
            // Client connected
            std::cout << "Client connected to named pipe." << std::endl;

            // Read from the named pipe
            char buffer[1024];
            DWORD bytesRead = 0;
            if (ReadFile(hPipe, buffer, sizeof(buffer), &bytesRead, nullptr))
            {
                std::cout << "Read " << bytesRead << " bytes from named pipe: " << buffer << std::endl;

                // Process the request from the client
                logger->get("CommandProcessor").debug("Read " + bytesRead + std::string(" bytes from named pipe: ") + buffer);
                std::ofstream ostr("C:\\Users\\benosa\\source\\repos\\KeyloggerService\\x64\\Debug\\NamedPipe.log");
                if (ostr.is_open()) {
                    ostr << "Read " << bytesRead << " bytes from named pipe: " << buffer << std::endl;
                    ostr.close();
                }
                if (std::string(buffer).find("stop") != std::string::npos) {
                    if (callback != NULL)callback();
                    logger->get("CommandProcessor").debug("Stopping the named pipe server.");
                    return 0;
                }
                // Write response back to the client
                /*if (!WriteFile(hPipe, "Response", sizeof("Response"), &bytesRead, nullptr))
                {
                    std::cerr << "Failed to write response to named pipe. Error code: " << GetLastError() << std::endl;
                }*/
            }
            else
            {
                switch (DWORD l = GetLastError()) {
                case ERROR_PIPE_CONNECTED: {
                        logger->get("CommandProcessor").error("Client already connected to named pipe.");
                        return 1;
                    }
                case ERROR_PIPE_LISTENING: {
                        logger->get("CommandProcessor").error("Named pipe is in listening mode.");
                        return 1;
                    }
                case ERROR_PIPE_NOT_CONNECTED: {
                        logger->get("CommandProcessor").error("Named pipe is closed.");
                        return 1;
                    }
                default: {
                        logger->get("CommandProcessor").error("Failed to connect named pipe with error " + std::to_string(l));
                        return 1;
                    }
                }
            }

            // Disconnect the client
            DisconnectNamedPipe(hPipe);
            CloseHandle(hPipe);
        }
        else
        {
            std::cerr << "Failed to connect named pipe. Error code: " << GetLastError() << std::endl;
            if (callback != NULL)callback();
            return 1;
        }
        //Sleep(1000);
    }

    return 0;
}

CommandProcessor::~CommandProcessor()
{
    if (hNamedPipe) {
        DisconnectNamedPipe(hNamedPipe);
        CloseHandle(hNamedPipe);
    }
}

//void CommandProcessor::send(wchar_t* command)
//{
//}
