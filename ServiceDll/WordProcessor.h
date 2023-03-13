#pragma once
#include <map>
#include <Windows.h>
#include "IWordProcessor.h"
#include "WindowInfo.h"
#include "IGuardProcessor.h"
#include "Poco/SharedPtr.h"
#include "AppContainer.h"
#include <Poco/Util/PropertyFileConfiguration.h>

typedef std::map<const HWND, Poco::SharedPtr<WindowInfo>> Mymap;

class WordProcessor :
    public IWordProcessor
{
private:
    Mymap mapWindows;
    IGuardProcessor* processor;
    bool editfeature;
public:
    WordProcessor(IGuardProcessor* processor);
    void process(HWND window, std::wstring str);
    int countWindow();
};

