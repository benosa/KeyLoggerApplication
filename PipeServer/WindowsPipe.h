#ifndef WINDOWSPIPE_H
#define WINDOWSPIPE_H

#include <windows.h>
#include <string>
#include <iostream>
#include <stdexcept>
#include <Poco/Pipe.h>

class WindowsPipe : public Poco::Pipe
{
public:
    enum class PipeMode {
        Server,
        Client
    };
    enum class PipeReadMode {
        NoWait = PIPE_TYPE_MESSAGE | PIPE_READMODE_MESSAGE | PIPE_NOWAIT,
        Wait = PIPE_TYPE_BYTE | PIPE_READMODE_BYTE | PIPE_WAIT
    };
    void setState(PipeReadMode lpMode);
    WindowsPipe(const std::string& name, PipeMode mode);
    void waitForConnection();
    int writeBytes(const void* buffer, int length);
    int readBytes(void* buffer, int length);
    HANDLE readHandle() const;
    HANDLE writeHandle() const;
    void closeRead();
    void closeWrite();
    ~WindowsPipe();

private:
    HANDLE m_readHandle;
    HANDLE m_writeHandle;
};

#endif // WINDOWSPIPE_H


//#pragma once
//#include "Windows.h"
//#include "Poco/Pipe.h"
//#include <stdexcept>
//
//class WindowsPipe : public Poco::Pipe
//{
//public:
//    enum class PipeMode {
//        Server,
//        Client
//    };
//
//    WindowsPipe(const std::string& name, PipeMode mode) : m_pipeName(name)
//    {
//        SECURITY_ATTRIBUTES saAttr = { 0 };
//        saAttr.nLength = sizeof(SECURITY_ATTRIBUTES);
//        saAttr.bInheritHandle = TRUE;
//        saAttr.lpSecurityDescriptor = NULL;
//
//        if (mode == PipeMode::Server) {
//            m_readHandle = CreateNamedPipeA(
//                name.c_str(),
//                PIPE_ACCESS_INBOUND | FILE_FLAG_OVERLAPPED,
//                PIPE_TYPE_BYTE | PIPE_READMODE_BYTE | PIPE_WAIT,
//                PIPE_UNLIMITED_INSTANCES,
//                BUFSIZ,
//                BUFSIZ,
//                0,
//                &saAttr);
//            if (m_readHandle == INVALID_HANDLE_VALUE) {
//                throw std::runtime_error("CreateNamedPipeA failed");
//            }
//
//            if (!ConnectNamedPipe(m_readHandle, NULL)) {
//                if (GetLastError() != ERROR_PIPE_CONNECTED) {
//                    throw std::runtime_error("ConnectNamedPipe failed");
//                }
//            }
//
//            m_writeHandle = CreateFileA(
//                name.c_str(),
//                GENERIC_WRITE,
//                0,
//                &saAttr,
//                OPEN_EXISTING,
//                FILE_ATTRIBUTE_NORMAL,
//                NULL);
//            if (m_writeHandle == INVALID_HANDLE_VALUE) {
//                throw std::runtime_error("CreateFileA failed");
//            }
//        }
//        else if (mode == PipeMode::Client) {
//            m_writeHandle = CreateFileA(
//                name.c_str(),
//                GENERIC_WRITE,
//                0,
//                &saAttr,
//                OPEN_EXISTING,
//                FILE_ATTRIBUTE_NORMAL,
//                NULL);
//            if (m_writeHandle == INVALID_HANDLE_VALUE) {
//                throw std::runtime_error("CreateFileA failed");
//            }
//
//            m_readHandle = CreateFileA(
//                name.c_str(),
//                GENERIC_READ,
//                0,
//                &saAttr,
//                OPEN_EXISTING,
//                FILE_ATTRIBUTE_NORMAL,
//                NULL);
//            if (m_readHandle == INVALID_HANDLE_VALUE) {
//                throw std::runtime_error("CreateFileA failed");
//            }
//        }
//        else {
//            throw std::invalid_argument("Invalid pipe mode");
//        }
//    }
//
//    void waitForConnection()
//    {
//        if (m_readHandle == INVALID_HANDLE_VALUE) {
//            throw std::runtime_error("Pipe is not open for reading");
//        }
//
//        OVERLAPPED overlapped = { 0 };
//        if (!ConnectNamedPipe(m_readHandle, &overlapped)) {
//            if (GetLastError() == ERROR_IO_PENDING) {
//                DWORD dwBytes;
//                BOOL bRet = GetOverlappedResult(m_readHandle, &overlapped, &dwBytes, TRUE);
//                if (!bRet) {
//                    throw std::runtime_error("GetOverlappedResult failed");
//                }
//            }
//            else {
//                throw std::runtime_error("ConnectNamedPipe failed");
//            }
//        }
//
//        m_writeHandle = CreateFileA(
//            m_pipeName.c_str(),
//            GENERIC_WRITE,
//            0,
//            NULL,
//            OPEN_EXISTING,
//            FILE_ATTRIBUTE_NORMAL | FILE_FLAG_OVERLAPPED,
//            NULL);
//        if (m_writeHandle == INVALID_HANDLE_VALUE) {
//            throw std::runtime_error("CreateFileA failed");
//        }
//    }
//
//    int writeBytes(const void* buffer, int length)
//    {
//        if (m_writeHandle == INVALID_HANDLE_VALUE) {
//            throw std::runtime_error("Pipe is not open for writing");
//        }
//
//        DWORD dwBytesWritten;
//        BOOL bRet = WriteFile(m_writeHandle, buffer, length, &dwBytesWritten, NULL);
//        if (!bRet) {
//            throw std::runtime_error("WriteFile failed");
//        }
//        return dwBytesWritten;
//    }
//
//    int readBytes(void* buffer, int length)
//    {
//        if (m_readHandle == INVALID_HANDLE_VALUE) {
//            throw std::runtime_error("Pipe is not open for reading");
//        }
//
//        DWORD dwBytesRead;
//        BOOL bRet = ReadFile(m_readHandle, buffer, length, &dwBytesRead, NULL);
//        if (!bRet) {
//            throw std::runtime_error("ReadFile failed");
//        }
//
//        return dwBytesRead;
//    }
//
//    HANDLE readHandle() const
//    {
//        return m_readHandle;
//    }
//
//    HANDLE writeHandle() const
//    {
//        return m_writeHandle;
//    }
//
//    void closeRead()
//    {
//        if (m_readHandle != INVALID_HANDLE_VALUE) {
//            CloseHandle(m_readHandle);
//            m_readHandle = INVALID_HANDLE_VALUE;
//        }
//    }
//
//    void closeWrite()
//    {
//        if (m_writeHandle != INVALID_HANDLE_VALUE) {
//            CloseHandle(m_writeHandle);
//            m_writeHandle = INVALID_HANDLE_VALUE;
//        }
//    }
//
//    ~WindowsPipe()
//    {
//        closeRead();
//        closeWrite();
//    }
//    private:
//        std::string m_pipeName;
//        HANDLE m_readHandle = INVALID_HANDLE_VALUE;
//        HANDLE m_writeHandle = INVALID_HANDLE_VALUE;
//};