#pragma once
#include <iostream>
#include <fstream>
#include <ctime>
#include <csignal>
#include "AdapterWorker.h"
#include "IKeyResolver.h"
#include "IWordProcessor.h"
#include <Poco/Logger.h>
#include "defines.h"
#include "HookThread.h"
#include "AppContainer.h"

class WorkerThread
{
private:
	IKeyResover* keyResolver;
	IWordProcessor* wordProcessor;
	//Poco::Util::Application* app;
	Poco::Logger* logger;
public:
	WorkerThread(IKeyResover* resolver, IWordProcessor* processor);
	void CreateHookThread();
	void static RemoveHookThread(int code);

};

