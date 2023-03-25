#include "IcuGuardProcessor.h"

size_t countWideCharacters(const std::string& str) {
    std::setlocale(LC_ALL, "en_US.UTF-8"); // or any other locale that supports your encoding
    size_t wideCharCount = 0;
    errno_t err = mbstowcs_s(&wideCharCount, nullptr, 0, str.c_str(), _TRUNCATE);

    if (err == 0) {
        return wideCharCount;
    }
    else {
        return 0;
    }
}


IcuGuardProcessor::IcuGuardProcessor(const std::string& jsonFilePath, Poco::Logger* _logger) {
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

std::string fixInvalidUtf82(const std::string& input, UChar32 replacement = 0xFFFD) {
    UErrorCode errorCode = U_ZERO_ERROR;
    UConverter* conv = ucnv_open("UTF-8", &errorCode);
    if (U_FAILURE(errorCode)) {
        std::cerr << "Error opening UTF-8 converter: " << u_errorName(errorCode) << std::endl;
        return std::string();
    }

    ucnv_setFromUCallBack(conv, UCNV_FROM_U_CALLBACK_SUBSTITUTE, &replacement, nullptr, nullptr, &errorCode);
    if (U_FAILURE(errorCode)) {
        std::cerr << "Error setting callback for UTF-8 converter: " << u_errorName(errorCode) << std::endl;
        ucnv_close(conv);
        return std::string();
    }

    icu::UnicodeString ustr(reinterpret_cast<const char*>(input.c_str()), input.length(), "UTF-8");
    //icu::UnicodeString ustr = icu::UnicodeString::fromUTF8(input);
    int32_t outputSize = ustr.length() * 4;
    std::string output(outputSize, '\0');

    int32_t actualSize = ucnv_fromUChars(conv, &output[0], outputSize, ustr.getBuffer(), ustr.length(), &errorCode);
    if (U_FAILURE(errorCode)) {
        std::cerr << "Error converting UnicodeString to UTF-8: " << u_errorName(errorCode) << std::endl;
        ucnv_close(conv);
        return std::string();
    }

    output.resize(actualSize);
    ucnv_close(conv);
    return output;
}

std::string fixInvalidUtf8(const std::string& input, UChar32 replacement = 0xFFFD) {
    UErrorCode errorCode = U_ZERO_ERROR;
    std::unique_ptr<icu::UnicodeString> ustr(
        icu::UnicodeString::fromUTF8(input).clone());
    std::string output;

    if (U_FAILURE(errorCode)) {
        icu::UnicodeString fixed;
        int32_t inputLen = static_cast<int32_t>(input.length());
        int32_t i = 0;
        while (i < inputLen) {
            UChar32 ch;
            U8_NEXT(input.data(), i, inputLen, ch);
            if (ch < 0) {
                fixed.append(replacement);
            }
            else {
                fixed.append(ch);
            }
        }
        fixed.toUTF8String(output);
    }
    else {
        output = input;
    }

    return output;
}

static std::string detectEncoding(const std::string& input) {
    UErrorCode status = U_ZERO_ERROR;
    UCharsetDetector* detector = ucsdet_open(&status);
    if (U_FAILURE(status)) {
        // Обработка ошибки открытия детектора
        return "";
    }

    ucsdet_setText(detector, input.c_str(), input.length(), &status);

    const UCharsetMatch* match = ucsdet_detect(detector, &status);
    if (U_FAILURE(status)) {
        // Обработка ошибки определения кодировки
        ucsdet_close(detector);
        return "";
    }

    const char* encoding = ucsdet_getName(match, &status);
    std::string detectedEncoding(encoding);

    ucsdet_close(detector);
    return detectedEncoding;
}

std::string IcuGuardProcessor::wstringToString(const std::wstring& wideStr) {
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

void IcuGuardProcessor::parseTree(Poco::JSON::Object::Ptr& root) {
    for (const auto& it : *root) {
        std::string word = it.first;
        std::string encoding = detectEncoding(word);
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

std::string IcuGuardProcessor::createPattern(const std::string& word) const {
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

bool IcuGuardProcessor::findPattern(const std::string& word, const std::string& stopWord) const {
    size_t cStopWord = countWideCharacters(stopWord);
    size_t cWord = countWideCharacters(word);
    if (cStopWord < cWord)return false;
    std::string fixed = fixInvalidUtf82(word);
    icu::UnicodeString uWord(fixed.c_str(), fixed.length());
    icu::UnicodeString uStopWord(stopWord.c_str(), stopWord.length());

    uWord = uWord.toLower();
    uStopWord = uStopWord.toLower();

    if (uStopWord.length() < uWord.length()) {
        return false;
    }

    int result = uWord.compare(uStopWord);
    if (result == 0)
        return true;
    /*if (uWord == uStopWord) {
        return true;
    }*/

    std::string fixed2 = fixInvalidUtf82(createPattern(word));
    //icu::UnicodeString pattern = icu::UnicodeString::fromUTF8(fixed2);
    icu::UnicodeString pattern(fixed2.c_str(), fixed2.length());
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

std::string IcuGuardProcessor::process(std::string stopWord, std::wstring stop) {
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
            if (findPattern(word, wstringToString(stop))) {
                return word;
            }
        }
        return "";
    }
}