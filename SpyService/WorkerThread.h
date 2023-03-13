#pragma once
#pragma once
#pragma comment (lib, "wtsapi32.lib")
#include <Poco/Logger.h>
#include <Poco/Thread.h>
#include <Poco/Util/Application.h>
#include <wtsapi32.h>
#include <windows.h>
#include <tlhelp32.h>
#include "InjectDll.h"

using namespace Poco;

class WorkerThread : public Poco::Runnable
{
public:
	WorkerThread(Poco::Util::Application* _app);
protected:
	virtual void run();
private:
	Poco::Util::Application* app;
	Logger* logger;
	LPCTSTR pBuf;
};

