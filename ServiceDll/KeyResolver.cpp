#include "KeyResolver.h"
#include "defines.h"
#ifdef TEST
extern int KEYBOARD_LANG;
#endif // TEST


std::wstring KeyResolver::resolveKey(int lang, int key, bool shift, bool capital) {
	std::wstring str = Keys::key(lang, key, shift, capital);
	return str;
}

std::wstring KeyResolver::resolve(int nCode, WPARAM wParam, LPARAM lParam) {
#ifdef TEST
		int keyLayout = KEYBOARD_LANG;
#else		
		HWND window = GetForegroundWindow();
		HKL keyboardLayout = GetKeyboardLayout(GetWindowThreadProcessId(window, NULL));
		int keyLayout = reinterpret_cast<int>(keyboardLayout);
#endif

    KBDLLHOOKSTRUCT* pKeyStruct = reinterpret_cast<KBDLLHOOKSTRUCT*>(lParam);
    std::wstring result;

    shift = GetKeyState(VK_SHIFT) & 0x8000;
    capital = (GetKeyState(VK_CAPITAL) & 0x0001) != 0;

    if (nCode == HC_ACTION && (wParam == WM_KEYDOWN || wParam == WM_SYSKEYDOWN))
    {
        result = resolveKey(keyLayout, pKeyStruct->vkCode, shift, capital);
    }

    return result;
}