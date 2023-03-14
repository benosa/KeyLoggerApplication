#pragma once
#pragma comment (lib, "wtsapi32.lib")
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
#include "InjectDll.h"

using namespace Poco;

struct EnumWindowsData
{
	HWND hwnd;
	const wchar_t* windowName;
};

class HookThread : public Poco::Runnable
{
public:
	HookThread(Poco::Util::Application* _app, IKeyResover* resolver, IWordProcessor* processor);
	void pipeServerThread();
	void sendCommand();
	void stop();
protected:
	void HookProc(KeyInfo receivedInfo);
	virtual void run();
	//~HookThread();
private:
	bool stopFlag = false;
	Poco::Util::Application* app;
	Logger* logger;
	IKeyResover* keyResolver;
	IWordProcessor* wordProcessor;
	LPCTSTR pBuf;
};

