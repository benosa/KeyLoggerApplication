#include "WorkerThread.h"

WorkerThread::WorkerThread(HANDLE done, IKeyResover* resolver, IWordProcessor* processor): doneEvent(&done), keyResolver(resolver), wordProcessor(processor){
	app = &Poco::Util::Application::instance();
	logger = &Poco::Logger::get("AppLogger");
}

void WorkerThread::CreateHookThread() {

	Poco::Util::Application* app = &Poco::Util::Application::instance();
	// Создаем поток и передаем ему указатель на логгер
	
	std::thread myThread([&]() {
		HookThread runnable(doneEvent, app, keyResolver, wordProcessor);
		Poco::Thread thread;
		thread.start(runnable);
		thread.join();
	});

	myThread.detach();
	logger->information("Message from main thread");

	return;
}
