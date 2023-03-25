#pragma once
#include <iostream>
#include <windows.h>
#include <winuser.h>
#include <fstream>
#include <ctime>
#include <csignal>
#include "defines.h"
#include "HookThread.h"
#include <thread>
#include "AdapterWorker.h"
#include "IKeyResolver.h"
#include "IWordProcessor.h"
#include <Poco/Logger.h>
#include <Poco/Util/Application.h>
#include <Poco/Util/ServerApplication.h>

class WorkerThread
{
private:
	HANDLE doneEvent;
	IKeyResover* keyResolver;
	IWordProcessor* wordProcessor;
	Poco::Util::Application* app;
	Poco::Logger* logger;
public:
	WorkerThread(HANDLE  doneEvent, IKeyResover* resolver, IWordProcessor* processor);
	void CreateHookThread();

};

