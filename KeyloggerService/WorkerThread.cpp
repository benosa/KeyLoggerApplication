#include "WorkerThread.h"
#include "defines.h"
#include "HookThread.h"

WorkerThread::WorkerThread(IKeyResover* resolver, IWordProcessor* processor): keyResolver(resolver), wordProcessor(processor){
	app = &Poco::Util::Application::instance();
	logger = &Poco::Logger::get("AppLogger");
}

void WorkerThread::RemoveHookThread(int code) {
	if (PostThreadMessageA(tid, WM_QUIT, NULL, NULL) == 0) {
		printf("\nCannot send the WM_QUIT message to the hook thread\n");
		Poco::Util::Application::instance().logger().error(std::string("WorkerThread::RemoveHookThread() : ") + "Cannot send the WM_QUIT message to the hook thread");
		exit(EXIT_FAILURE);
	}
}

void WorkerThread::CreateHookThread() {

	Poco::Util::Application* app = &Poco::Util::Application::instance();
	// Создаем поток и передаем ему указатель на логгер
	HookThread runnable(app, keyResolver, wordProcessor);

	Poco::Thread thread;

	thread.start(runnable);

	logger->information("Message from main thread");

	thread.join();

	return;
}
