#pragma once
#include <Poco/SharedPtr.h>
#include <Poco/Logger.h>
#include <Poco/Util/PropertyFileConfiguration.h>
#include <Poco/FileChannel.h>
#include <Poco/PatternFormatter.h>
#include <Poco/FormattingChannel.h>
using namespace Poco;
class AppContainer {
public:
    static AppContainer& instance() {
        static AppContainer container;
        return container;
    }

    Poco::AutoPtr<Poco::Util::AbstractConfiguration> config() const {
        return _config;
    }

    Poco::AutoPtr<Poco::Logger> appLogger() const {
        return _appLogger;
    }

    Poco::AutoPtr<Poco::Logger> keyLogger() const {
        return _keyLogger;
    }

private:
    AppContainer() {
        // создайте и загрузите контейнер здесь
        std::string appPath = "C:\\Users\\benosa\\source\\repos\\KeyloggerService\\x64\\Debug\\";

        _config = new Poco::Util::PropertyFileConfiguration(appPath + "KeyloggerService.properties");

        std::string loggerName = _config->getString("application.logger.name");
        std::string logLevel = _config->getString("application.logger.level", "debug");
        std::string logFile = _config->getString("application.logger.file", "mylog.log");
        std::string rotationLog = _config->getString("application.logger.rotation");
        std::string patternLog = _config->getString("application.logger.pattern");

        std::string keylogLevel = _config->getString("application.keylogger.level", "debug");
        std::string keylogFile = _config->getString("application.keylogger.file", "mylog.log");
        std::string keyrotationLog = _config->getString("application.keylogger.rotation");
        std::string keypatternLog = _config->getString("application.keylogger.pattern");

        /*
        * Создание логера приложения
        */

        Poco::AutoPtr<Poco::FileChannel> pChannel(new Poco::FileChannel);
        pChannel->setProperty("path", appPath + logFile);
        pChannel->setProperty("rotation", "2 K");
        pChannel->setProperty("archive", "timestamp");
        Poco::AutoPtr<Poco::PatternFormatter> pPatern(new Poco::PatternFormatter);
        pPatern->setProperty("pattern", patternLog);
        Poco::AutoPtr<Poco::FormattingChannel> pAppLoggerFormatterChannel(new Poco::FormattingChannel(pPatern, pChannel));       
        _appLogger = &Logger::get("AppLogger");
        _appLogger->setChannel(pAppLoggerFormatterChannel);
        _appLogger->setLevel(logLevel);

        /*_appLogger = &Logger::get("CommandProcessor");
        _appLogger->setChannel(pAppLoggerFormatterChannel);
        _appLogger->setLevel(logLevel);*/
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
             
        _keyLogger = &Logger::get("KeyLogger");
        _keyLogger->setChannel(pKeyloggerLoggerFormatterChannel);
        _keyLogger->setLevel(logLevel);
    }

    Poco::AutoPtr<Poco::Util::AbstractConfiguration> _config;
    Poco::AutoPtr<Poco::Logger> _appLogger;
    Poco::AutoPtr<Poco::Logger> _keyLogger;
};