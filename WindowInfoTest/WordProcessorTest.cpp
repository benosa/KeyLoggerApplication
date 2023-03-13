#include "CppUnitTest.h"
#include "GuardProcessorMock.h"
#include "WordProcessor.h"
#include "Poco/SharedPtr.h"
#include <wchar.h>
using namespace testing;

using namespace Microsoft::VisualStudio::CppUnitTestFramework;

namespace WindowInfoTest
{
    TEST_CLASS(WordProcessorTest)
    {
    private:
        std::wstring title = L"Test Window";
        GuardProcessorMock* mockGuardProcessor = new GuardProcessorMock();
        WordProcessor* wp = new WordProcessor(mockGuardProcessor);
        static LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
        {
            switch (message)
            {
            case WM_MYMESSAGE:
                // обработка сообщений для нашего виртуального окна
                break;
            default:
                return DefWindowProc(hWnd, message, wParam, lParam);
            }
            return 0;
        };
        HWND hWnd;
    public:
        TEST_METHOD_INITIALIZE(start) {
            HMODULE hModule = GetModuleHandle(NULL);
            WNDCLASS wc = { 0 };
            wc.lpfnWndProc = WndProc;
            wc.hInstance = hModule;
            wc.lpszClassName = L"MyClass";
            RegisterClass(&wc);

            // создание окна
            hWnd = CreateWindow(L"MyClass", L"MyWindow", WS_OVERLAPPEDWINDOW,
                CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT,
                NULL, NULL, hModule, NULL);
        }

        TEST_METHOD(TestProcess_AddCharToExistingWindow)
        {
            wp->process(hWnd, L"Q");

            Assert::AreEqual(1, wp->countWindow());
        }
    };
}