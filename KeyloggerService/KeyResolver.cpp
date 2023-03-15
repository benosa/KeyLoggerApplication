#include "KeyResolver.h"
#include "defines.h"
#ifdef TEST
extern int KEYBOARD_LANG;
#endif // TEST


std::wstring KeyResolver::resolveKey(int lang, int key, bool shift, bool capital) {
	std::wstring str = Keys::key(lang, key, shift, capital);
	return str;
}

std::wstring KeyResolver::resolve(int lang, int vkCode, bool shift, bool capital) {
#ifdef TEST
		int keyLayout = KEYBOARD_LANG;
#else		
		/*HWND window = GetForegroundWindow();
		HKL keyboardLayout = GetKeyboardLayout(GetWindowThreadProcessId(window, NULL));
		int keyLayout = reinterpret_cast<int>(keyboardLayout);*/
    int keyLayout = lang;
#endif
    std::wstring result;
	result = resolveKey(keyLayout, vkCode, shift, capital);
    return result;
}

/*
lang,
key,
shift,
capital
*/