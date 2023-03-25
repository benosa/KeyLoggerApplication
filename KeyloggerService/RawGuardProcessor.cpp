#include "RawGuardProcessor.h"
#include <fstream>
#include <locale>
#include <algorithm>
#include <ranges>
#include <charconv>
#include <codecvt>

RawGuardProcessor::RawGuardProcessor(const std::string& jsonFilePath, Poco::Logger* _logger) {
    logger = _logger;
    std::ifstream jsonFile(jsonFilePath);
    if (!jsonFile.is_open()) {
        std::cerr << "Ошибка открытия файла: " << jsonFilePath << std::endl;
        return;
    }
    Poco::JSON::Parser parser;
    Poco::Dynamic::Var result = parser.parse(jsonFile);
    Poco::JSON::Object::Ptr root = result.extract<Poco::JSON::Object::Ptr>();
    parseTree(root);
}

std::string RawGuardProcessor::wstringToString(const std::wstring& wideStr) {
    std::u32string utf32Str(wideStr.begin(), wideStr.end());
    std::vector<char> utf8Buffer(utf32Str.size() * 4);
    char* out = utf8Buffer.data();

    for (auto c : utf32Str) {
        out = std::to_chars(out, out + 4, c, 16).ptr;
    }

    return std::string(utf8Buffer.data(), out);
}

//std::wstring RawGuardProcessor::stringToWstring(const std::string& inputStr) {
//    std::wstring_convert<std::codecvt_utf8<wchar_t>, wchar_t> convert;
//    return convert.from_bytes(inputStr);
//}

void RawGuardProcessor::parseTree(Poco::JSON::Object::Ptr& root) {
    for (const auto& it : *root) {
        std::string word = it.first;
        Poco::Dynamic::Var element = root->get(word);

        if (element.type() == typeid(Poco::JSON::Array::Ptr)) {
            Poco::JSON::Array::Ptr subWords = element.extract<Poco::JSON::Array::Ptr>();
            m_tree[word] = {};
            for (const auto& subWord : *subWords) {
                m_tree[word].push_back(subWord.toString());
            }
        }
        else {
            m_tree[word] = {};
        }
    }
}

bool RawGuardProcessor::matchPattern(const std::string& pattern, const std::string& text) {
    auto p = pattern.begin();
    auto t = text.begin();

    while (p != pattern.end() && t != text.end()) {
        if (*p == '*') {
            if (++p == pattern.end()) {
                return true;
            }
            t = std::find(t, text.end(), *p);
        }
        else if (*p == '?' || std::tolower(*p) == std::tolower(*t)) {
            ++p;
            ++t;
        }
        else {
            return false;
        }
    }

    return p == pattern.end() && t == text.end();
}

std::string RawGuardProcessor::process(std::string stopWord, std::wstring stop) {
    std::string stopStr = wstringToString(stop);
    std::string lowerStopStr;
    std::ranges::transform(stopStr, std::back_inserter(lowerStopStr), ::tolower);

    if (stopWord.size() > 0) {
        const auto stopSubArray = m_tree.find(wstringToString(stop));
        if (stopSubArray == m_tree.end()) return "";
        if (stopSubArray->second.size() == 0) return "";
        for (const auto& it : stopSubArray->second) {
            std::string lowerWord;
            std::ranges::transform(it, std::back_inserter(lowerWord), ::tolower);
            if (matchPattern(lowerWord, lowerStopStr)) {
                return stopWord;
            }
        }
        return "";
    }
    else {
        for (const auto& it : m_tree) {
            std::string lowerWord;
            std::ranges::transform(it.first, std::back_inserter(lowerWord), ::tolower);
            if (matchPattern(lowerWord, lowerStopStr)) {
                return it.first;
            }
            for (const auto& subWord : it.second) {
                std::string lowerSubWord;
                std::ranges::transform(subWord, std::back_inserter(lowerSubWord), ::tolower);
                if (matchPattern(lowerSubWord, lowerStopStr)) {
                    return it.first;
                }
            }
        }
        return "";
    }
}