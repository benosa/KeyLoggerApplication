#pragma once
#include <sstream>

class IGuardProcessor {
public:
	virtual bool process(bool stopStatus, std::wstring sequence) = 0;
};