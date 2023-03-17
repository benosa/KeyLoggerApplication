#include "RawGuardProcessor.h"

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
    UErrorCode errorCode = U_ZERO_ERROR;
    UChar* uWideStr = reinterpret_cast<UChar*>(const_cast<wchar_t*>(wideStr.c_str()));
    int32_t uWideStrLength = static_cast<int32_t>(wideStr.length());

    int32_t utf8Length;
    u_strToUTF8(nullptr, 0, &utf8Length, uWideStr, uWideStrLength, &errorCode);
    errorCode = U_ZERO_ERROR;

    std::string utf8Str(utf8Length, '\0');
    u_strToUTF8(&utf8Str[0], utf8Length, nullptr, uWideStr, uWideStrLength, &errorCode);

    if (U_FAILURE(errorCode)) {
        logger->error("Ошибка конвертации строки: " + std::string(u_errorName(errorCode)));
        return std::string();
    }

    return utf8Str;
}

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

std::string RawGuardProcessor::createPattern(const std::string& word) const {
    std::string pattern = word;
    size_t pos = 0;

    // Заменяем все символы '*' на '.*' в pattern
    while ((pos = pattern.find('*', pos)) != std::string::npos) {
        pattern.replace(pos, 1, ".*");
        pos += 2;
    }

    pos = 0;
    // Заменяем все символы '?' на '.' в pattern
    while ((pos = pattern.find('?', pos)) != std::string::npos) {
        pattern.replace(pos, 1, ".");
        pos += 1;
    }

    // Возвращаем регулярное выражение, созданное на основе шаблона
    return pattern;
}

bool RawGuardProcessor::findPattern(const std::string& word, const std::string& stopWord) const {
    icu::UnicodeString uWord = icu::UnicodeString::fromUTF8(word);
    icu::UnicodeString uStopWord = icu::UnicodeString::fromUTF8(stopWord);

    uWord = uWord.toLower();
    uStopWord = uStopWord.toLower();

    if (uStopWord.length() < uWord.length()) {
        return false;
    }

    if (uWord == uStopWord) {
        return true;
    }

    icu::UnicodeString pattern = icu::UnicodeString::fromUTF8(createPattern(word));
    UErrorCode status = U_ZERO_ERROR;
    icu::RegexMatcher* matcher = new icu::RegexMatcher(pattern, uStopWord, 0, status);

    if (U_FAILURE(status)) {
        delete matcher;
        logger->error("Ошибка регулярного выражения: " + std::string(u_errorName(status)));
        return false;
    }

    bool foundMatch = false;

    while (matcher->find(status)) {
        foundMatch = true;
        // Выполнение действий с найденным совпадением, если необходимо
    }

    delete matcher;

    return foundMatch;
}

std::string RawGuardProcessor::process(std::string stopWord, std::wstring stop) {
    if (stopWord.size() > 0) {
        const auto stopSubArray = m_tree.find(wstringToString(stop));
        if (stopSubArray == m_tree.end()) return "";
        if (stopSubArray->second.size() == 0)return "";
        for (const auto& it : stopSubArray->second) {
            std::string word = it;
            if (findPattern(word, stopWord)) {
                return stopWord;
            }
        }
        return "";
    }
    else {
        for (const auto& it : m_tree) {
            std::string word = it.first;
            if (findPattern(word, stopWord)) {
                return stopWord;
            }
        }
        return "";
    }
}