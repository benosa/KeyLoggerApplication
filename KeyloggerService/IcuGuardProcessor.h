#pragma once

#include <fstream>
#include <iostream>
#include <string>
#include <map>
#include <vector>
#include <unicode/regex.h>
#include <unicode/unistr.h>
#include <unicode/utypes.h>
#include <unicode/ustring.h>
#include <unicode/uchar.h>
#include <unicode/ucsdet.h>
#include <unicode/ucnv.h>

#include <Poco/Util/Application.h>
#include "Poco/JSON/Parser.h"
#include "IGuardProcessor.h"
#include <Poco/Logger.h>

class IcuGuardProcessor :
    public IGuardProcessor
{
public:
    IcuGuardProcessor(const std::string& jsonFilePath, Poco::Logger* logger);
    std::string process(std::string stopWord, std::wstring sequence);
private:
    Poco::Logger* logger;
    std::map<std::string, std::vector<std::string>> m_tree;

    void parseTree(Poco::JSON::Object::Ptr& root);
    bool findPattern(const std::string& word, const std::string& stopWord) const;
    std::string createPattern(const std::string& word) const;
    std::string wstringToString(const std::wstring& wideStr);
};

