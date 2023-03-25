#include "WindowInfo.h"


WindowInfo::WindowInfo(Poco::Util::Application* _app, std::wstring title, IGuardProcessor* processor) : app(_app), guard(processor) {
    this->title = title;
    loggerApp = &app->logger().get("AppLogger");
    logger = &app->logger().get("KeyLogger");
    //logger = &Poco::Logger::get("KeyLogger");
    //logger = &Poco::Logger::get("AppLogger");
    //loggerApp = &Poco::Logger::get("AppLogger");
    wordbuffer = app->config().getInt("application.keylogger.wordbuffer", 2048);
    //wchar_t BOM = static_cast<wchar_t>(0xFEFF);
    //std::locale loc(std::locale(), new std::codecvt_utf16<wchar_t>);
    //std::locale loc(std::locale(), new std::codecvt<wchar_t, char, std::mbstate_t>);
    //std::locale loc("");
    //stream.imbue(loc);
    //stream << BOM;
    caret = 0;
    //countWords = 0;
}
std::string WindowInfo::wstringToString(std::wstring str) {
    std::string _title;
    Poco::UnicodeConverter::toUTF8(str, _title);
    return _title;
}


bool WindowInfo::getAlarmState() {
    return alarmState;
}

void WindowInfo::moveLeft() {
    if (caret == 1)return;
    caret--;
}

void WindowInfo::moveRight() {
    if (caret == stream.str().size())return;
    caret++;
}

void WindowInfo::clearStream() {
    stopStatus = false;
    caret = 0;
    stream.str(std::wstring());
    stream.clear();
}

void WindowInfo::removeChar(int direction) {
    std::wstring s = stream.str();
    if (direction == -1) {
        if (caret == (s.size() > 1)?1:0)return;
        s.erase(--caret, 1);
    }
    else if(direction == 1) {
        //caret всегда 1
        //s.size() > 1
        //но если size = 1, to caret = 0;
        if ((caret == s.size()) && caret != 1)return;
        s.erase(caret-1, 1);
        if (s.size() == 0)caret = 0;
    }
    else {
        std::string str("WindowInfo::removeChar(): ");
        //loggerApp->error(str + "Error: Direction");
        throw std::runtime_error("Error: Direction");
    }
    loggerApp->debug("WindowInfo::beforeRemoveChar(): " + wstringToString(stream.str()));
    stream.str(std::wstring());
    stream.clear();
    stream << s; // записываем измененную строку обратно в поток
    //loggerApp->debug("WindowInfo::afterRemoveChar(): " + wstringToString(stream.str()));
}

void WindowInfo::addChar(std::wstring str) {
    loggerApp->debug("WindowInfo::addChar(): " + wstringToString(str));
    if (str.size() == 0 || str.size() > 1)return;
    stream.flush();
    stream.put(str.c_str()[0]);
    ++caret;
    //если у нас уже статус alarmState == true
    if (alarmState) {
        //скинем буфер в лог если он уже большой
        if (stream.str().size() > wordbuffer) {
            stopStatus = false;
            caret = 0;
            alarmState = 0;
            logger->log(wstringToString(title) + ": " + wstringToString(stream.str()));
            stream.str(std::wstring());
            stream.clear();
           
        }
    }
    //если у нас статус alarmState == false
    else {
        // с каждым поступлением символа на вход, запрашиваем у модуля GuardModule статус присутствия
        // стоп слов или подслов
        std::string result = guard->process((stopStatus)? stopWord:"", stream.str());
        stopWord = result;
        bool guardResult = (result.size() > 0) ? true : false;
        // guardResult && stopStatus -> alarmState = true
        // !guardResult && stopStatus -> stopStatus = true, alarmState = false
        // guardResult && !stopStatus -> stopStatus = true, alarmState = false
        // !guardResult && !stopStatus -> stopStatus = false, alarmState = false
        //alarmState = guardResult && stopStatus ? true : ((!guardResult && !stopStatus)? stopStatus = false : stopStatus = true, false);

        if (guardResult) {
            if (stopStatus) {
                if (!alarmState)
                    alarmState = true;
            }
            else {
                stopStatus = true;
            }
        }

        if (!alarmState && !stopStatus) {
            if(stream.str().size() > wordbuffer) {
                stopStatus = false;
                caret = 0;
                stream.str(std::wstring());
                stream.clear();
            }
        }
    }
}

std::wstring WindowInfo::getTitle() {
    return title;
}

std::wstringstream& WindowInfo::getStream() {
    return stream;
}

int WindowInfo::getCaret() {
    return caret;
}

bool WindowInfo::getStopStatus() {
    return stopStatus;
}
