#pragma once
#include <Windows.h>
#include <WinError.h>
#include <fstream>
#include <Poco/Logger.h>

#include "ICommandProcessor.h"
#include "AdapterWorker.h"


class CommandProcessor: public ICommandProcessor {
public:
	//typedef void (*StopCallback)(void);

	CommandProcessor(wchar_t pipePath[], Poco::Logger logger/*, StopCallback callback*/);
	//StopCallback callback;
	//void send(wchar_t* command);
private:
	bool stopFlag;
	HANDLE hNamedPipe;
	Poco::Logger& logger;

	DWORD WINAPI NamedPipeServerThread(LPVOID lpParam);
	void setStopPipeFlag();

	~CommandProcessor();
 };