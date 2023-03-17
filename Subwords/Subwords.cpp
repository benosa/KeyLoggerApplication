#include <fstream>
#include <iostream>
#include <string>
#include <vector>
#include "Poco/JSON/Parser.h"
#include <codecvt>
#include <regex>
#include <map>
#include <filesystem>
#include <locale>
#include <codecvt>
#include <cwctype>
#include <unicode/regex.h>
#include <unicode/unistr.h>

class JsonTree {
public:
    
    JsonTree(const std::string& jsonFilePath) {
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

    void parseTree(Poco::JSON::Object::Ptr& root) {
        for (const auto& it : *root) {
            std::string word = it.first;
            Poco::Dynamic::Var element = root->get(word);

            if  (element.type() == typeid(Poco::JSON::Array::Ptr)) {
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

    std::string createPattern(const std::string& word) const {
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

    bool findPattern(const std::string& word, const std::string& stopWord) const {
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
            std::cerr << "Ошибка регулярного выражения: " << u_errorName(status) << std::endl;
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

    void printJsonTree() {
        Poco::JSON::Object jsonTree;
        for (const auto& it : m_tree) {
            Poco::JSON::Array subWords;
            for (const auto& subWord : it.second) {
                subWords.add(subWord);
            }
            jsonTree.set(it.first, subWords);
        }
        std::stringstream ss;
        Poco::JSON::Stringifier::stringify(jsonTree, ss);
        std::cout << ss.str() << std::endl;
    }

    bool containsStopWord(const std::string& stopWord) const {
        for (const auto& it : m_tree) {
            std::string word = it.first;
            if (findPattern(word, stopWord)) {
                return true;
            }
        }
        return false;
    }

    bool containsStopSubWord(const std::string& stopWord, const std::string& stop) const {
        const auto stopSubArray = m_tree.find(stop);
        if (stopSubArray == m_tree.end()) return false;
        if (stopSubArray->second.size() == 0)return false;
        for (const auto& it : stopSubArray->second) {
            std::string word = it;
            if (findPattern(word, stopWord)) {
                return true;
            }
        }
        return false;
    }

private:
    std::map<std::string, std::vector<std::string>> m_tree;
};

int main() {
    JsonTree tree("C:\\Users\\benosa\\source\\repos\\KeyloggerService\\x64\\Debug\\tree.json");
    tree.printJsonTree();
    std::cout << std::boolalpha;

    //должны найти шаблон "?резиден*" 
    std::cout << "Test 1: " << (tree.containsStopWord("ОпреПрезидентамихотимчто-то мне сделать") ? "Passed" : "Failed") << std::endl;
    //ничего не должны найти
    std::cout << "Test 2: " << (tree.containsStopWord("Опреентамихотимчто-то мне сделать") ? "Failed" : "Passed") << std::endl;
    //должны найти шаблон "бо*ба"
    std::cout << "Test 3: " << (tree.containsStopSubWord("ОпреПрезидентамБохотимчто-томба мне сделать",  "?резиден*") ? "Passed" : "Failed") << std::endl;
    //не должны найти шаблон "бо?ба"
    std::cout << "Test 4: " << (tree.containsStopSubWord("ОпреПрентамБохотимчто-томба мне сделать", "?резен*") ? "Failed" : "Passed") << std::endl;
    //должны найти "бо?ба"
    std::cout << "Test 5: " << (tree.containsStopSubWord("ОпреПрентамБохотимчто-болба мне сделать", "?резен*") ? "Passed" : "Failed") << std::endl;
    return 0;
}