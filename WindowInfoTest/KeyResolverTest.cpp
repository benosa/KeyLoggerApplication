#include "CppUnitTest.h"
#include "KeyResolver.h"
#include <cwctype>
#include "defines.h"

#ifdef TEST
int KEYBOARD_LANG = 67699721;
#endif // TEST


using namespace Microsoft::VisualStudio::CppUnitTestFramework;
/// <summary>
/// ��� ����� ����������� ���������� � ����� defines.h ���������� TEST � 1 
/// ����� ��� ����� ����� ���� ���������, �������� ���������� NUM PAD
/// </summary>
namespace KeyResolverTests
{
    TEST_CLASS(KeyResolverTests)
    {
    private:
        KeyResolver* resolver;
    public:
        TEST_METHOD_INITIALIZE(start) {
            resolver = new KeyResolver();
        }
        TEST_METHOD(TestResolveEnglishLetters)
        {
            /*
            *   a-z
            */
            for (int keyCode = 0x41; keyCode <= 0x5A; keyCode++){
                KBDLLHOOKSTRUCT keystroke;
                keystroke.vkCode = keyCode;
                keystroke.scanCode = MapVirtualKey(keyCode, MAPVK_VK_TO_VSC);
                keystroke.flags = LLKHF_INJECTED;
                keystroke.time = 0;
                keystroke.dwExtraInfo = 0;
                /*keystroke.dwExtraInfo |= (GetKeyState(VK_CAPITAL) & 0x0001) ? 0x01 : 0x00;
                keystroke.dwExtraInfo |= (GetKeyState(VK_LSHIFT) & 0x8000) ? 0x02 : 0x00;*/
                keystroke.dwExtraInfo |= 0x00;
                keystroke.dwExtraInfo |= 0x00;

                PKBDLLHOOKSTRUCT pKeystroke = &keystroke;

                std::wstring expected = std::wstring(1, static_cast<wchar_t>(keyCode));
                expected = std::towlower(expected.c_str()[0]);
                Assert::AreEqual(expected, resolver->resolve(0, WM_KEYDOWN, reinterpret_cast<LPARAM>(&keystroke)));
            }
            /*
            *   SHIFT a-z
            */
            for (int keyCode = 0x41; keyCode <= 0x5A; keyCode++) {
                KBDLLHOOKSTRUCT keystroke;
                keystroke.vkCode = keyCode;
                keystroke.scanCode = MapVirtualKey(keyCode, MAPVK_VK_TO_VSC);
                keystroke.flags = LLKHF_INJECTED;
                keystroke.time = 0;
                keystroke.dwExtraInfo = 0;
                keystroke.dwExtraInfo |= 0x00;
                keystroke.dwExtraInfo |= 0x02;

                PKBDLLHOOKSTRUCT pKeystroke = &keystroke;

                std::wstring expected = std::wstring(1, static_cast<wchar_t>(keyCode));
                expected = std::towupper(expected.c_str()[0]);
                Assert::AreEqual(expected, resolver->resolve(0, WM_KEYDOWN, reinterpret_cast<LPARAM>(&keystroke)));
            }
            /*
            *   CAPS a-z
            */
            for (int keyCode = 0x41; keyCode <= 0x5A; keyCode++) {
                KBDLLHOOKSTRUCT keystroke;
                keystroke.vkCode = keyCode;
                keystroke.scanCode = MapVirtualKey(keyCode, MAPVK_VK_TO_VSC);
                keystroke.flags = LLKHF_INJECTED;
                keystroke.time = 0;
                keystroke.dwExtraInfo = 0;
                keystroke.dwExtraInfo |= 0x01;
                keystroke.dwExtraInfo |= 0x00;

                PKBDLLHOOKSTRUCT pKeystroke = &keystroke;

                std::wstring expected = std::wstring(1, static_cast<wchar_t>(keyCode));
                expected = std::towupper(expected.c_str()[0]);
                Assert::AreEqual(expected, resolver->resolve(0, WM_KEYDOWN, reinterpret_cast<LPARAM>(&keystroke)));
            }
            /*
            *  SHIFT + CAPS a-z
            */
            for (int keyCode = 0x41; keyCode <= 0x5A; keyCode++) {
                KBDLLHOOKSTRUCT keystroke;
                keystroke.vkCode = keyCode;
                keystroke.scanCode = MapVirtualKey(keyCode, MAPVK_VK_TO_VSC);
                keystroke.flags = LLKHF_INJECTED;
                keystroke.time = 0;
                keystroke.dwExtraInfo = 0;
                keystroke.dwExtraInfo |= 0x01;
                keystroke.dwExtraInfo |= 0x02;

                PKBDLLHOOKSTRUCT pKeystroke = &keystroke;

                std::wstring expected = std::wstring(1, static_cast<wchar_t>(keyCode));
                expected = std::towlower(expected.c_str()[0]);
                Assert::AreEqual(expected, resolver->resolve(0, WM_KEYDOWN, reinterpret_cast<LPARAM>(&keystroke)));
            }
            /*
            *   0-9
            */
            for (int keyCode = 0x30; keyCode <= 0x39; keyCode++) {
                KBDLLHOOKSTRUCT keystroke;
                keystroke.vkCode = keyCode;
                keystroke.scanCode = MapVirtualKey(keyCode, MAPVK_VK_TO_VSC);
                keystroke.flags = LLKHF_INJECTED;
                keystroke.time = 0;
                keystroke.dwExtraInfo = 0;
                /*keystroke.dwExtraInfo |= (GetKeyState(VK_CAPITAL) & 0x0001) ? 0x01 : 0x00;
                keystroke.dwExtraInfo |= (GetKeyState(VK_LSHIFT) & 0x8000) ? 0x02 : 0x00;*/
                keystroke.dwExtraInfo |= 0x00;
                keystroke.dwExtraInfo |= 0x00;

                PKBDLLHOOKSTRUCT pKeystroke = &keystroke;

                std::wstring expected = std::wstring(1, static_cast<wchar_t>(keyCode));
                expected = std::towlower(expected.c_str()[0]);
                Assert::AreEqual(expected, resolver->resolve(0, WM_KEYDOWN, reinterpret_cast<LPARAM>(&keystroke)));
            }
            /*
            *   SHIFT 0-9
            */
            for (int keyCode = 0x30, i = 0; keyCode <= 0x39; keyCode++, i++) {
                wchar_t str[10] = {L')',L'!',L'@',L'#',L'$',L'%',L'^',L'&',L'*',L'('};
                KBDLLHOOKSTRUCT keystroke;
                keystroke.vkCode = keyCode;
                keystroke.scanCode = MapVirtualKey(keyCode, MAPVK_VK_TO_VSC);
                keystroke.flags = LLKHF_INJECTED;
                keystroke.time = 0;
                keystroke.dwExtraInfo = 0;
                /*keystroke.dwExtraInfo |= (GetKeyState(VK_CAPITAL) & 0x0001) ? 0x01 : 0x00;
                keystroke.dwExtraInfo |= (GetKeyState(VK_LSHIFT) & 0x8000) ? 0x02 : 0x00;*/
                keystroke.dwExtraInfo |= 0x00;
                keystroke.dwExtraInfo |= 0x02;

                PKBDLLHOOKSTRUCT pKeystroke = &keystroke;

                std::wstring expected = std::wstring(1, static_cast<wchar_t>(keyCode));
                expected = str[i];
                Assert::AreEqual(expected, resolver->resolve(0, WM_KEYDOWN, reinterpret_cast<LPARAM>(&keystroke)));
            }
            /*
            *   OEM
            */
            int oem[11] = { VK_OEM_1, VK_OEM_2 , VK_OEM_3, VK_OEM_4, VK_OEM_5, VK_OEM_6, VK_OEM_7, VK_OEM_PLUS, VK_OEM_COMMA, VK_OEM_MINUS, VK_OEM_PERIOD };
            wchar_t strNormal[11] = { L';',L'/',L'`',L'[',L'\\',L']',L'\'',L'=',L',',L'-',L'.' };
            wchar_t strShift[11]  = { L':',L'?',L'~',L'{',L'|', L'}',L'\"',L'+',L'<',L'_',L'>' };
            for (int i = 0; i < 11 ; i++) {  
                KBDLLHOOKSTRUCT keystroke;
                keystroke.vkCode = oem[i];
                keystroke.scanCode = MapVirtualKey(oem[i], MAPVK_VK_TO_VSC);
                keystroke.flags = LLKHF_INJECTED;
                keystroke.time = 0;
                keystroke.dwExtraInfo = 0;
                keystroke.dwExtraInfo |= 0x00;
                keystroke.dwExtraInfo |= 0x00;

                PKBDLLHOOKSTRUCT pKeystroke = &keystroke;

                std::wstring expected = std::wstring(1, static_cast<wchar_t>(oem[i]));
                expected = strNormal[i];
                Assert::AreEqual(expected, resolver->resolve(0, WM_KEYDOWN, reinterpret_cast<LPARAM>(&keystroke)));
            }
            /*
            *   OEM SHIFT
            */
            for (int i = 0; i < 11; i++) {
                KBDLLHOOKSTRUCT keystroke;
                keystroke.vkCode = oem[i];
                keystroke.scanCode = MapVirtualKey(oem[i], MAPVK_VK_TO_VSC);
                keystroke.flags = LLKHF_INJECTED;
                keystroke.time = 0;
                keystroke.dwExtraInfo = 0;
                keystroke.dwExtraInfo |= 0x00;
                keystroke.dwExtraInfo |= 0x02;

                PKBDLLHOOKSTRUCT pKeystroke = &keystroke;

                std::wstring expected = std::wstring(1, static_cast<wchar_t>(oem[i]));
                expected = strShift[i];
                Assert::AreEqual(expected, resolver->resolve(0, WM_KEYDOWN, reinterpret_cast<LPARAM>(&keystroke)));
            }
        }
        
        TEST_METHOD(TestResolveRussianLetters)
        {
            KEYBOARD_LANG = 68748313;
            /*
            *   �-�
            */
            wchar_t rus[26] = { L'�',L'�',L'�',L'�',L'�',L'�',L'�',L'�',L'�',L'�',L'�',L'�',L'�',L'�',L'�',L'�',L'�',L'�',L'�',L'�',L'�',L'�',L'�',L'�',L'�',L'�'};
            wchar_t rusUpper[26] = { L'�',L'�',L'�',L'�',L'�',L'�',L'�',L'�',L'�',L'�',L'�',L'�',L'�',L'�',L'�',L'�',L'�',L'�',L'�',L'�',L'�',L'�',L'�',L'�',L'�',L'�' };
            wchar_t strNormal[11] = { L';',L'/',L'`',L'[',L'\\',L']',L'\'',L'=',L',',L'-',L'.' };
            wchar_t strShift[11] = { L':',L'?',L'~',L'{',L'|', L'}',L'\"',L'+',L'<',L'_',L'>' };

            for (int keyCode = 0x41, i = 0; keyCode <= 0x5A; keyCode++, i++) {
                KBDLLHOOKSTRUCT keystroke;
                keystroke.vkCode = keyCode;
                keystroke.scanCode = MapVirtualKey(keyCode, MAPVK_VK_TO_VSC);
                keystroke.flags = LLKHF_INJECTED;
                keystroke.time = 0;
                keystroke.dwExtraInfo = 0;
                /*keystroke.dwExtraInfo |= (GetKeyState(VK_CAPITAL) & 0x0001) ? 0x01 : 0x00;
                keystroke.dwExtraInfo |= (GetKeyState(VK_LSHIFT) & 0x8000) ? 0x02 : 0x00;*/
                keystroke.dwExtraInfo |= 0x00;
                keystroke.dwExtraInfo |= 0x00;

                PKBDLLHOOKSTRUCT pKeystroke = &keystroke;

                std::wstring expected = std::wstring(1, static_cast<wchar_t>(keyCode));
                expected = std::towlower(rus[i]);
                Assert::AreEqual(expected, resolver->resolve(0, WM_KEYDOWN, reinterpret_cast<LPARAM>(&keystroke)));
            }
            /*
            *   SHIFT a-z
            */
            for (int keyCode = 0x41, i = 0; keyCode <= 0x5A; keyCode++,i++) {
                KBDLLHOOKSTRUCT keystroke;
                keystroke.vkCode = keyCode;
                keystroke.scanCode = MapVirtualKey(keyCode, MAPVK_VK_TO_VSC);
                keystroke.flags = LLKHF_INJECTED;
                keystroke.time = 0;
                keystroke.dwExtraInfo = 0;
                keystroke.dwExtraInfo |= 0x00;
                keystroke.dwExtraInfo |= 0x02;

                PKBDLLHOOKSTRUCT pKeystroke = &keystroke;

                std::wstring expected = std::wstring(1, rusUpper[i]);
                Assert::AreEqual(expected, resolver->resolve(0, WM_KEYDOWN, reinterpret_cast<LPARAM>(&keystroke)));
            }
            /*
            *   CAPS a-z
            */
            for (int keyCode = 0x41, i=0; keyCode <= 0x5A; keyCode++,i++) {
                KBDLLHOOKSTRUCT keystroke;
                keystroke.vkCode = keyCode;
                keystroke.scanCode = MapVirtualKey(keyCode, MAPVK_VK_TO_VSC);
                keystroke.flags = LLKHF_INJECTED;
                keystroke.time = 0;
                keystroke.dwExtraInfo = 0;
                keystroke.dwExtraInfo |= 0x01;
                keystroke.dwExtraInfo |= 0x00;

                PKBDLLHOOKSTRUCT pKeystroke = &keystroke;

                std::wstring expected = std::wstring(1, rusUpper[i]);
                Assert::AreEqual(expected, resolver->resolve(0, WM_KEYDOWN, reinterpret_cast<LPARAM>(&keystroke)));
            }
            /*
            *  SHIFT + CAPS a-z
            */
            for (int keyCode = 0x41, i =0; keyCode <= 0x5A; keyCode++, i++) {
                KBDLLHOOKSTRUCT keystroke;
                keystroke.vkCode = keyCode;
                keystroke.scanCode = MapVirtualKey(keyCode, MAPVK_VK_TO_VSC);
                keystroke.flags = LLKHF_INJECTED;
                keystroke.time = 0;
                keystroke.dwExtraInfo = 0;
                keystroke.dwExtraInfo |= 0x01;
                keystroke.dwExtraInfo |= 0x02;

                PKBDLLHOOKSTRUCT pKeystroke = &keystroke;

                std::wstring expected = std::wstring(1, rus[i]);
                Assert::AreEqual(expected, resolver->resolve(0, WM_KEYDOWN, reinterpret_cast<LPARAM>(&keystroke)));
            }
            /*
            *   0-9
            */
            for (int keyCode = 0x30; keyCode <= 0x39; keyCode++) {
                KBDLLHOOKSTRUCT keystroke;
                keystroke.vkCode = keyCode;
                keystroke.scanCode = MapVirtualKey(keyCode, MAPVK_VK_TO_VSC);
                keystroke.flags = LLKHF_INJECTED;
                keystroke.time = 0;
                keystroke.dwExtraInfo = 0;
                /*keystroke.dwExtraInfo |= (GetKeyState(VK_CAPITAL) & 0x0001) ? 0x01 : 0x00;
                keystroke.dwExtraInfo |= (GetKeyState(VK_LSHIFT) & 0x8000) ? 0x02 : 0x00;*/
                keystroke.dwExtraInfo |= 0x00;
                keystroke.dwExtraInfo |= 0x00;

                PKBDLLHOOKSTRUCT pKeystroke = &keystroke;

                std::wstring expected = std::wstring(1, static_cast<wchar_t>(keyCode));
                expected = std::towlower(expected.c_str()[0]);
                Assert::AreEqual(expected, resolver->resolve(0, WM_KEYDOWN, reinterpret_cast<LPARAM>(&keystroke)));
            }
            /*
            *   SHIFT 0-9
            */
            for (int keyCode = 0x30, i = 0; keyCode <= 0x39; keyCode++, i++) {
                wchar_t str[10] = { L')',L'!',L'\"',L'�',L';',L'%',L':',L'?',L'*',L'(' };
                KBDLLHOOKSTRUCT keystroke;
                keystroke.vkCode = keyCode;
                keystroke.scanCode = MapVirtualKey(keyCode, MAPVK_VK_TO_VSC);
                keystroke.flags = LLKHF_INJECTED;
                keystroke.time = 0;
                keystroke.dwExtraInfo = 0;
                /*keystroke.dwExtraInfo |= (GetKeyState(VK_CAPITAL) & 0x0001) ? 0x01 : 0x00;
                keystroke.dwExtraInfo |= (GetKeyState(VK_LSHIFT) & 0x8000) ? 0x02 : 0x00;*/
                keystroke.dwExtraInfo |= 0x00;
                keystroke.dwExtraInfo |= 0x02;

                PKBDLLHOOKSTRUCT pKeystroke = &keystroke;

                std::wstring expected = std::wstring(1, static_cast<wchar_t>(keyCode));
                expected = str[i];
                Assert::AreEqual(expected, resolver->resolve(0, WM_KEYDOWN, reinterpret_cast<LPARAM>(&keystroke)));
            }
            /*
            *   OEM
            */
            int oem[11] = { VK_OEM_1, VK_OEM_2 , VK_OEM_3, VK_OEM_4, VK_OEM_5, VK_OEM_6, VK_OEM_7, VK_OEM_PLUS, VK_OEM_COMMA, VK_OEM_MINUS, VK_OEM_PERIOD };
            wchar_t oemLow[11] = { L'�',L'.',L'�',L'�',L'\\', L'�',L'�',L'=',L'�',L'-',L'�' };
            for (int i = 0; i < 11; i++) {
                KBDLLHOOKSTRUCT keystroke;
                keystroke.vkCode = oem[i];
                keystroke.scanCode = MapVirtualKey(oem[i], MAPVK_VK_TO_VSC);
                keystroke.flags = LLKHF_INJECTED;
                keystroke.time = 0;
                keystroke.dwExtraInfo = 0;
                keystroke.dwExtraInfo |= 0x00;
                keystroke.dwExtraInfo |= 0x00;

                PKBDLLHOOKSTRUCT pKeystroke = &keystroke;

                std::wstring expected = std::wstring(1, static_cast<wchar_t>(oem[i]));
                expected = oemLow[i];
                Assert::AreEqual(expected, resolver->resolve(0, WM_KEYDOWN, reinterpret_cast<LPARAM>(&keystroke)));
            }
            /*
            *   OEM SHIFT
            */
            wchar_t oemUpper[11] = { L'�',L',',L'�',L'�',L'/', L'�',L'�',L'+',L'�',L'_',L'�' };
            for (int i = 0; i < 11; i++) {
                KBDLLHOOKSTRUCT keystroke;
                keystroke.vkCode = oem[i];
                keystroke.scanCode = MapVirtualKey(oem[i], MAPVK_VK_TO_VSC);
                keystroke.flags = LLKHF_INJECTED;
                keystroke.time = 0;
                keystroke.dwExtraInfo = 0;
                keystroke.dwExtraInfo |= 0x00;
                keystroke.dwExtraInfo |= 0x02;

                PKBDLLHOOKSTRUCT pKeystroke = &keystroke;

                std::wstring expected = std::wstring(1, oemUpper[i]);
                Assert::AreEqual(expected, resolver->resolve(0, WM_KEYDOWN, reinterpret_cast<LPARAM>(&keystroke)));
            }
        }
    };
}
