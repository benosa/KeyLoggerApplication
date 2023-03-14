#ifndef WINDOWSPIPE_H
#define WINDOWSPIPE_H

#include <windows.h>
#include <string>
#include <iostream>
#include <stdexcept>
#include <Poco/Pipe.h>

class WindowsPipe: public Poco::Pipe
{
public:
    enum class PipeMode {
        Server,
        Client
    };
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