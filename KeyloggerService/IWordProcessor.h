#pragma once
#include <string>
#include <Poco/Util/Application.h>

class IWordProcessor {
public:
	virtual void process(Poco::Util::Application* _app, HWND window, std::wstring str) = 0;
	virtual int countWindow() = 0;
};