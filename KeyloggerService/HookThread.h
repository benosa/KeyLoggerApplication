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
	HookThread(HANDLE doneEvent, Poco::Util::Application* _app, IKeyResover* resolver, IWordProcessor* processor);
	DWORD WINAPI pipeServerThread(LPVOID lpThreadParameter, std::wstring name);
	void sendCommand();
	void stop();
protected:
	void HookProc(KeyInfo receivedInfo);
	virtual void run();
	//~HookThread();
private:
	HANDLE doneEvent;
	HANDLE serverPipe;
	Poco::Util::Application* app;
	Logger* logger;
	IKeyResover* keyResolver;
	IWordProcessor* wordProcessor;
	LPCTSTR pBuf;
};

