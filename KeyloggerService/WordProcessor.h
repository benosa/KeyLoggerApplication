#pragma once
#include <map>
#include <Windows.h>
#include "IWordProcessor.h"
#include "WindowInfo.h"
#include "IGuardProcessor.h"
#include "Poco/SharedPtr.h"

typedef std::map<const HWND, Poco::SharedPtr<WindowInfo>> Mymap;

class WordProcessor :
    public IWordProcessor
{
private:
    Poco::Util::Application* app;
    Mymap mapWindows;
    IGuardProcessor* processor;
    bool editfeature;
public:
    WordProcessor(IGuardProcessor* processor);
    void process(Poco::Util::Application* _app, HWND window, std::string text, std::wstring str);
    size_t countWindow();
};

