#include "WindowsPipe.h"
#include <Windows.h>
#include <Psapi.h>
#include <Shlwapi.h>
#include <Aclapi.h>

///* Create a DACL that will allow everyone to have full control over our pipe. */
//static VOID BuildDACL(PSECURITY_DESCRIPTOR pDescriptor)
//{
//    PSID pSid;
//    EXPLICIT_ACCESS ea;
//    PACL pAcl;
//
//    SID_IDENTIFIER_AUTHORITY sia = SECURITY_WORLD_SID_AUTHORITY;
//
//    AllocateAndInitializeSid(&sia, 1, SECURITY_WORLD_RID, 0, 0, 0, 0, 0, 0, 0,
//        &pSid);
//
//    ZeroMemory(&ea, sizeof(EXPLICIT_ACCESS));
//    ea.grfAccessPermissions = FILE_ALL_ACCESS;
//    ea.grfAccessMode = SET_ACCESS;
//    ea.grfInheritance = NO_INHERITANCE;
//    ea.Trustee.TrusteeForm = TRUSTEE_IS_SID;
//    ea.Trustee.TrusteeType = TRUSTEE_IS_WELL_KNOWN_GROUP;
//    ea.Trustee.ptstrName = (LPTSTR)pSid;
//
//    if (SetEntriesInAcl(1, &ea, NULL, &pAcl) == ERROR_SUCCESS)
//    {
//        if (SetSecurityDescriptorDacl(pDescriptor, TRUE, pAcl, FALSE) == 0)
//            throw std::runtime_error("Failed to create named pipe");
//            //_tprintf(_T("[*] Failed to set DACL (%u)\n"), GetLastError());
//    }
//    else
//        throw std::runtime_error("Failed to create named pipe");
//        //_tprintf(_T("[*] Failed to add ACE in DACL (%u)\n"), GetLastError());
//}
//
//
///* Create a SACL that will allow low integrity processes connect to our pipe. */
//static VOID BuildSACL(PSECURITY_DESCRIPTOR pDescriptor)
//{
//    PSID pSid;
//    PACL pAcl;
//
//    SID_IDENTIFIER_AUTHORITY sia = SECURITY_MANDATORY_LABEL_AUTHORITY;
//    DWORD dwACLSize = sizeof(ACL) + sizeof(SYSTEM_MANDATORY_LABEL_ACE) +
//        GetSidLengthRequired(1);
//
//    pAcl = (PACL)LocalAlloc(LPTR, dwACLSize);
//    InitializeAcl(pAcl, dwACLSize, ACL_REVISION);
//
//    AllocateAndInitializeSid(&sia, 1, SECURITY_MANDATORY_LOW_RID, 0, 0, 0, 0,
//        0, 0, 0, &pSid);
//
//    if (AddMandatoryAce(pAcl, ACL_REVISION, 0, SYSTEM_MANDATORY_LABEL_NO_WRITE_UP,
//        pSid) == TRUE)
//    {
//        if (SetSecurityDescriptorSacl(pDescriptor, TRUE, pAcl, FALSE) == 0)
//            throw std::runtime_error("Failed to create named pipe");
//            //_tprintf(_T("[*] Failed to set SACL (%u)\n"), GetLastError());
//    }
//    else
//        throw std::runtime_error("Failed to create named pipe");
//        //_tprintf(_T("[*] Failed to add ACE in SACL (%u)\n"), GetLastError());
//}
//
//
///* Initialize security attributes to be used by `CreateNamedPipe()' below. */
//static VOID InitSecurityAttributes(PSECURITY_ATTRIBUTES pAttributes)
//{
//    PSECURITY_DESCRIPTOR pDescriptor;
//
//    pDescriptor = (PSECURITY_DESCRIPTOR)LocalAlloc(LPTR,
//        SECURITY_DESCRIPTOR_MIN_LENGTH);
//    InitializeSecurityDescriptor(pDescriptor, SECURITY_DESCRIPTOR_REVISION);
//
//    BuildDACL(pDescriptor);
//    BuildSACL(pDescriptor);
//
//    pAttributes->nLength = sizeof(SECURITY_ATTRIBUTES);
//    pAttributes->lpSecurityDescriptor = pDescriptor;
//    pAttributes->bInheritHandle = TRUE;
//}

WindowsPipe::WindowsPipe(const std::string& name, PipeMode mode)
{
    std::string _str = "\\\\.\\pipe\\" + name;

    if (mode == PipeMode::Server) {
        SECURITY_ATTRIBUTES sa;
        HANDLE hPipe;
        /*DWORD dwRead, error;
        COMMTIMEOUTS timeouts;
        TCHAR szBuffer[BUFSIZ + 1];*/

        //HANDLE evt = (HANDLE)lpThreadParameter;
        DWORD dwModeNoWait = PIPE_TYPE_MESSAGE | PIPE_READMODE_MESSAGE | PIPE_NOWAIT;
        DWORD dwModeWait = PIPE_TYPE_BYTE | PIPE_READMODE_BYTE | PIPE_WAIT;

        DWORD ret = EXIT_FAILURE;


        //_tprintf(_T("[*] Starting server at %s\n"), PIPE_NAME);


        InitSecurityAttributes(&sa);

        hPipe = CreateNamedPipe(
            std::wstring(_str.begin(), _str.end()).c_str(),
            PIPE_ACCESS_DUPLEX, 
            dwModeWait,
            PIPE_UNLIMITED_INSTANCES, 
            PAGE_SIZE, 
            PAGE_SIZE, 
            INFINITE, 
            &sa);

       /* HANDLE hPipe = CreateNamedPipe(
            std::wstring(_str.begin(), _str.end()).c_str(),
            PIPE_ACCESS_DUPLEX,
            dwModeWait,
            PIPE_UNLIMITED_INSTANCES,
            INFINITE,
            NULL);*/

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
    if (!ConnectNamedPipe(m_readHandle, NULL)) {
        int i = GetLastError();
        throw std::runtime_error("Failed to wait for client connection");
    }
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