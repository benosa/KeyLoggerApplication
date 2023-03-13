#pragma once
#include <wchar.h>
#include <stdio.h>

class ICommandProcessor {
public:
	virtual void send(wchar_t command) = 0;
};