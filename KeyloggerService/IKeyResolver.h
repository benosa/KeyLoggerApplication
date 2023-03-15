#pragma once
#include <Windows.h>

class IKeyResover {
public:
	virtual std::wstring resolve(int lang, int vkCode, bool shift, bool capital) = 0;
};