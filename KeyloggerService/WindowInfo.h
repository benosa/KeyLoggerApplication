#pragma once
#include <string>
#include <sstream>
#include <locale>
#include <codecvt>
#include <Poco/Util/Application.h>
#include <Poco/UnicodeConverter.h>
#include "Poco/Util/LoggingSubsystem.h"
#include "defines.h"
#include "IGuardProcessor.h"

using Poco::Util::Application;

class WindowInfo {
private:
    Poco::Util::Application* app;
    Poco::Logger* logger;
    Poco::Logger* loggerApp;
    int wordbuffer;
    std::wstring title;
    std::wstringstream stream;
    int caret;
    bool stopStatus;
    bool alarmState;
    //int countWords;
    IGuardProcessor* guard;
    std::string wstringToString(std::wstring str);
public:
    WindowInfo(Poco::Util::Application* _app, std::wstring title, IGuardProcessor* processor);
    std::wstring getTitle();
    std::wstringstream& getStream();
    void clearStream();
    int getCaret();
    bool getStopStatus();
    bool getAlarmState();
    void addChar(std::wstring str);
    //-1 backspace; 1 delete
    void removeChar(int direction);
    void moveLeft();
    void moveRight();
};