#pragma once
#include <Poco/Logger.h>
#include <Poco/Thread.h>
#include <Poco/Util/Application.h>
#include <wtsapi32.h>
#include <windows.h>
#include <tlhelp32.h>
#include "AdapterWorker.h"
#include "IKeyResolver.h"
#include "IWordProcessor.h"
#include "defines.h"
#include "CommandProcessor.h"
#include <Poco/UnicodeConverter.h>
#include <tchar.h>
#include <format>
#include "AppContainer.h"

using namespace Poco;

struct EnumWindowsData
{
	HWND hwnd;
	const wchar_t* windowName;
};

class HookThread : public Poco::Runnable
{
public:
	HookThread(IKeyResover* resolver, IWordProcessor* processor);
protected:
	LRESULT CALLBACK  HookProc(int nCode, WPARAM wParam, LPARAM lParam);
	virtual void run();
private:
	void stop();
	Logger* logger;
	IKeyResover* keyResolver;
	IWordProcessor* wordProcessor;
	LPCTSTR pBuf;
	HHOOK hHook;
};

