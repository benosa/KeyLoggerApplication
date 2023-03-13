#pragma once
#include <iostream>
#include <windows.h>
#include <fstream>
#include <ctime>
#include <csignal>
#include <sstream>
#include "Poco/NumberFormatter.h"
#include "Poco/Util/Application.h"
#include "AdapterWorker.h"
#include "IKeyResolver.h"
#include "KeyConstants.h"

class KeyResolver : public IKeyResover
{
private:
	bool capital = false;
	bool numLock = false;
	bool shift = false;

	std::wstring resolveKey(int lang, int key, bool shift, bool capital);
public:
	std::wstring resolve(int nCode, WPARAM wParam, LPARAM lParam);
};

