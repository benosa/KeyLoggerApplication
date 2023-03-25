#pragma once
//#pragma comment (lib, "wtsapi32.lib")
#include "defines.h"
#include <Poco/Logger.h>
#include <Poco/Thread.h>
#include <Poco/Util/Application.h>
#include <wtsapi32.h>
#include <windows.h>
#include <tlhelp32.h>
#include "AdapterWorker.h"
#include <Poco/UnicodeConverter.h>
#include <tchar.h>
#include <format>
#include <Poco/PipeStream.h>
#include "WindowsPipe.h"
#include <Poco/Runnable.h>
#include <Poco/RunnableAdapter.h>
#include <thread>
#include "IKeyResolver.h"
#include "IWordProcessor.h"
//#include "InjectDll.h"
#include "Injector.h"


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
	void sendCommand(std::string command);
	void stop();
	void start();
protected:
	void HookProc(KeyInfo receivedInfo);
	virtual void run();
	//~HookThread();
private:
	bool createWritePipe();
	std::string servicePipe;
	std::string dllPipe;
	HANDLE doneEvent;
	HANDLE serverPipe;
	Poco::Util::Application* app;
	Logger* logger;
	IKeyResover* keyResolver;
	IWordProcessor* wordProcessor;
	LPCTSTR pBuf;
};

