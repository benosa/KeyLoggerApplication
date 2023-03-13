#pragma once
#include <iostream>
#pragma  comment(lib, "user32")
#pragma  comment(lib, "advapi32")
#include <windows.h>
#include <winuser.h>
#include <fstream>
#include <ctime>
#include <csignal>
#include "AdapterWorker.h"
#include "IKeyResolver.h"
#include "IWordProcessor.h"
#include <Poco/Logger.h>
#include <Poco/Util/Application.h>
#include <Poco/Util/ServerApplication.h>

class WorkerThread
{
private:
	inline static DWORD tid = 0;
	IKeyResover* keyResolver;
	IWordProcessor* wordProcessor;
	Poco::Util::Application* app;
	Poco::Logger* logger;
	//void HookJob();
	//LRESULT CALLBACK HookProc(int nCode, WPARAM wParam, LPARAM lParam);
public:
	WorkerThread(IKeyResover* resolver, IWordProcessor* processor);
	void CreateHookThread();
	void static RemoveHookThread(int code);

};

