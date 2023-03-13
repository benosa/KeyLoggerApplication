#pragma once
#include <Windows.h>
#include <WinError.h>
#include <fstream>
#include <iostream>
#include <Poco/Logger.h>
#include "AdapterWorker.h"
#include <functional>


class CommandProcessor {
public:
	typedef void (*StopCallback)(void);

	CommandProcessor(std::wstring, Poco::Logger* logger, std::function<void()> callback);
	std::function<void()> callback;
	//void send(wchar_t* command);
	~CommandProcessor();
private:
	bool stopFlag;
	HANDLE hNamedPipe;
	Poco::Logger* logger;

	DWORD WINAPI NamedPipeServerThread(LPVOID lpParam);
	void setStopPipeFlag();

	
 };