#include "includes.h"
#include "pipe.h"
#include <iostream>
#include "WindowsPipe.h"
#include <Poco/PipeStream.h>

int main()
{
    struct MyData {
        int a;
        double b;
        char c[256];
    };

    const std::string pipeName = "\\\\.\\pipe\\injectdll1";

   HANDLE hServerThread;
    HANDLE doneEvent;

    doneEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
    hServerThread = StartPipeServerThread(doneEvent);
    while (true) {
        Sleep(1000);
    }
    SetEvent(doneEvent);
    WaitForSingleObject(hServerThread, INFINITE);
    CloseHandle(hServerThread);

    std::cout << "Hello World!\n";
}

// Запуск программы: CTRL+F5 или меню "Отладка" > "Запуск без отладки"
// Отладка программы: F5 или меню "Отладка" > "Запустить отладку"

// Советы по началу работы 
//   1. В окне обозревателя решений можно добавлять файлы и управлять ими.
//   2. В окне Team Explorer можно подключиться к системе управления версиями.
//   3. В окне "Выходные данные" можно просматривать выходные данные сборки и другие сообщения.
//   4. В окне "Список ошибок" можно просматривать ошибки.
//   5. Последовательно выберите пункты меню "Проект" > "Добавить новый элемент", чтобы создать файлы кода, или "Проект" > "Добавить существующий элемент", чтобы добавить в проект существующие файлы кода.
//   6. Чтобы снова открыть этот проект позже, выберите пункты меню "Файл" > "Открыть" > "Проект" и выберите SLN-файл.
