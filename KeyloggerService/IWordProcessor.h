#pragma once
#include <string>
#include <Poco/Util/Application.h>

class IWordProcessor {
public:
	virtual void process(Poco::Util::Application* _app, HWND window, std::string text, std::wstring str) = 0;
	virtual size_t countWindow() = 0;
};