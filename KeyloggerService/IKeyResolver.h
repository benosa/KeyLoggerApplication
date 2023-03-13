#pragma once
#include <Windows.h>

class IKeyResover {
public:
	virtual std::wstring resolve(int nCode, WPARAM wParam, LPARAM lParam) = 0;
};