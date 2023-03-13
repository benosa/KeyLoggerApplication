#include "WorkerThread.h"

WorkerThread::WorkerThread(IKeyResover* resolver, IWordProcessor* processor): keyResolver(resolver), wordProcessor(processor){
	logger = AppContainer::instance().appLogger();
}

void WorkerThread::CreateHookThread() {

	// Создаем поток и передаем ему указатель на логгер
	HookThread runnable(keyResolver, wordProcessor);
	Poco::Thread thread;
	thread.start(runnable);
	logger->information("Message from main thread");
	thread.join();
	return;
}
