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
	HANDLE doneEvent;
	IKeyResover* keyResolver;
	IWordProcessor* wordProcessor;
	Poco::Util::Application* app;
	Poco::Logger* logger;
public:
	WorkerThread(HANDLE  doneEvent, IKeyResover* resolver, IWordProcessor* processor);
	void CreateHookThread();

};

