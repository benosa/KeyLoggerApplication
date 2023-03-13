#include "WorkerThread.h"

WorkerThread::WorkerThread(IKeyResover* resolver, IWordProcessor* processor): keyResolver(resolver), wordProcessor(processor){
	//app = &Poco::Util::Application::instance();
	//logger = &Poco::Logger::get("AppLogger");
	logger = AppContainer::instance().appLogger();
}

void WorkerThread::CreateHookThread() {

	//Poco::Util::Application* app = &Poco::Util::Application::instance();
	// Создаем поток и передаем ему указатель на логгер
	HookThread runnable(keyResolver, wordProcessor);

	Poco::Thread thread;

	thread.start(runnable);

	logger->information("Message from main thread");

	thread.join();

	return;
}
