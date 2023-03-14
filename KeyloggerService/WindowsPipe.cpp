#include "WindowsPipe.h"

WindowsPipe::WindowsPipe(const std::string& name, PipeMode mode)
{
    /*std::string _str = "\\\\.\\pipe\\" + name;
    HANDLE hPipe = CreateNamedPipe(
        std::wstring(_str.begin(), _str.end()).c_str(),
        PIPE_ACCESS_DUPLEX | FILE_FLAG_OVERLAPPED,
        PIPE_TYPE_BYTE | PIPE_READMODE_BYTE | PIPE_WAIT,
        PIPE_UNLIMITED_INSTANCES,
        0,
        0,
        INFINITE,
        NULL);

    if (hPipe == INVALID_HANDLE_VALUE)
        throw std::runtime_error("Failed to create named pipe");

    m_readHandle = hPipe;
    m_writeHandle = hPipe;*/
    std::string _str = "\\\\.\\pipe\\" + name;

    if (mode == PipeMode::Server) {
        HANDLE hPipe = CreateNamedPipe(
            std::wstring(_str.begin(), _str.end()).c_str(),
            PIPE_ACCESS_DUPLEX /* | FILE_FLAG_OVERLAPPED*/,
            PIPE_TYPE_BYTE | PIPE_READMODE_BYTE | PIPE_WAIT,
            PIPE_UNLIMITED_INSTANCES,
            0,
            0,
            INFINITE,
            NULL);

        if (hPipe == INVALID_HANDLE_VALUE)
            throw std::runtime_error("Failed to create named pipe");

        m_readHandle = hPipe;
        m_writeHandle = hPipe;
    }
    else if (mode == PipeMode::Client) {
        HANDLE hPipe = CreateFile(
            std::wstring(_str.begin(), _str.end()).c_str(),
            GENERIC_READ | GENERIC_WRITE,
            0,
            NULL,
            OPEN_EXISTING,
            0,
            NULL);

        if (hPipe == INVALID_HANDLE_VALUE)
            throw std::runtime_error("Failed to open named pipe");

        m_readHandle = hPipe;
        m_writeHandle = hPipe;
    }
    else {
        throw std::runtime_error("Invalid pipe mode");
    }
}

void WindowsPipe::waitForConnection()
{
    if (!ConnectNamedPipe(m_readHandle, NULL))
        throw std::runtime_error("Failed to wait for client connection");
}

int WindowsPipe::writeBytes(const void* buffer, int length)
{
    DWORD bytesWritten = 0;
    if (!WriteFile(m_writeHandle, buffer, length, &bytesWritten, NULL))
        throw std::runtime_error("Failed to write to named pipe");

    return bytesWritten;
}

int WindowsPipe::readBytes(void* buffer, int length)
{
    DWORD bytesRead = 0;
    if (!ReadFile(m_readHandle, buffer, length, &bytesRead, NULL))
        throw std::runtime_error("Failed to read from named pipe");

    return bytesRead;
}

HANDLE WindowsPipe::readHandle() const
{
    return m_readHandle;
}

HANDLE WindowsPipe::writeHandle() const
{
    return m_writeHandle;
}

void WindowsPipe::closeRead()
{
    CloseHandle(m_readHandle);
    m_readHandle = NULL;
}

void WindowsPipe::closeWrite()
{
    CloseHandle(m_writeHandle);
    m_writeHandle = NULL;
}

WindowsPipe::~WindowsPipe()
{
    if (m_readHandle != NULL)
        CloseHandle(m_readHandle);

    if (m_writeHandle != NULL)
        CloseHandle(m_writeHandle);
}