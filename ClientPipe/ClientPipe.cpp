// ClientPipe.cpp : Этот файл содержит функцию "main". Здесь начинается и заканчивается выполнение программы.
//

#include <iostream>
#include <Windows.h>
#include <tchar.h>

#define PIPE_NAME _T("\\\\.\\pipe\\injectdll1")

struct MyData {
    int a;
    double b;
    char c[256];
};

int main()
{
    HANDLE hPipe;
    DWORD dwBytesWritten;
    CHAR szBuffer[BUFSIZ + 1];

    hPipe = CreateFile(PIPE_NAME, GENERIC_READ | GENERIC_WRITE, 0, NULL, OPEN_EXISTING, 0, NULL);
    if (hPipe == INVALID_HANDLE_VALUE)
    {
        std::cerr << "Failed to open pipe (" << GetLastError() << ")\n";
        return 1;
    }

    MyData data;
    data.a = 42;
    data.b = 3.14;
    strcpy_s(data.c, "Hello, world!");

    while (true) {
        DWORD bytesWritten;
        if (!WriteFile(hPipe, &data, sizeof(data), &bytesWritten, NULL)) {
            std::cerr << "Failed to write to pipe (" << GetLastError() << ")\n";
            CloseHandle(hPipe);
            return 1;
        }
        Sleep(100);
    }

   /* std::wstring wstr;

    std::cout << "Enter message to send to server:\n";
    std::cin.getline(szBuffer, BUFSIZ);*/

   /* if (!WriteFile(hPipe, szBuffer, strlen(szBuffer) * sizeof(TCHAR), &dwBytesWritten, NULL))
    {
        std::cerr << "Failed to write to pipe (" << GetLastError() << ")\n";
        CloseHandle(hPipe);
        return 1;
    }*/

    std::cout << "Message sent to server!\n";

    CloseHandle(hPipe);
    return 0;
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
