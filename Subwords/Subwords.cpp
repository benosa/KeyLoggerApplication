#include <fstream>
#include <iostream>
#include <string>
#include <vector>

#include "Poco/JSON/Parser.h"
#include "Poco/Dynamic/Var.h"
#include "Poco/JSON/Object.h"
#include "Poco/JSON/Array.h"
#include "Poco/JSON/Stringifier.h"
#include "Poco/JSON/JSONException.h"
#include <Poco/UnicodeConverter.h>
#include <Poco/UTFString.h>
#include <Poco/FileStream.h>
#include <regex>

#include <map>
#include <filesystem>
#include <locale>
#include <codecvt>
#include <cwctype>
#define ENCODING_ASCII      0
#define ENCODING_UTF8       1
#define ENCODING_UTF16LE    2
#define ENCODING_UTF16BE    3
#define _CRT_SECURE_NO_WARNINGS 1

#include <Poco/FileStream.h>
#include <Poco/TextConverter.h>
#include <Poco/UTF8Encoding.h>
#include <Poco/UTF32Encoding.h>
#include <Poco/Windows1251Encoding.h>
#include <Poco/TextIterator.h>

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

    /*bool findPattern(const std::string& word, const std::string& stopWord) const {
        if (stopWord.size() < word.size()) {
            return false;
        }

        if (word == stopWord) {
            return true;
        }

        if (word.find("*") != std::string::npos) {
            std::string left = word.substr(0, word.find("*"));
            std::string right = word.substr(word.find("*") + 1);
            size_t leftPos = stopWord.find(left);
            size_t rightPos = 0;

            do {
                if (leftPos == std::string::npos) {
                    break;
                }
                rightPos = stopWord.find(right, leftPos + left.size());
                if (rightPos != std::string::npos) {
                    return true;
                }
                leftPos = stopWord.find(left, leftPos + left.size());
            } while (leftPos != std::string::npos);
        }
        else if (word.find("?") != std::string::npos) {
            std::string left = word.substr(0, word.find("?"));
            std::string right = word.substr(word.find("?") + 1);

            for (size_t i = 0; i < stopWord.size() - word.size() + 1; ++i) {
                if (stopWord.substr(i, left.size()) == left && stopWord.substr(i + left.size() + 1, right.size()) == right) {
                    return true;
                }
            }
        }

        return false;
    }*/
    static std::string toLower(const std::string& str) {
        std::setlocale(LC_ALL, "en_US.utf8"); // or another appropriate locale
        std::wstring_convert<std::codecvt_utf8<wchar_t>> conv;
        std::wstring wideStr = conv.from_bytes(str);
        std::transform(wideStr.begin(), wideStr.end(), wideStr.begin(),
            [](wchar_t c) { return std::towlower(c); });
        return conv.to_bytes(wideStr);
    }

    bool findPattern(const std::string& word, const std::string& stopWord) const {
        std::string lowerWord = toLower(word);
        std::string lowerStopWord = toLower(stopWord);
        // Если размер stopWord меньше размера word, возвращаем false
        if (lowerStopWord.size() < word.size()) {
            return false;
        }

        // Если слова совпадают, возвращаем true
        if (lowerWord == lowerStopWord) {
            return true;
        }

        std::string pattern = lowerWord;
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

        try {
            // Создаем объект регулярного выражения re на основе pattern
            std::regex re(pattern);
            // Если stopWord соответствует регулярному выражению re, возвращаем true
            if (std::regex_search(lowerStopWord, re)) {
                return true;
            }
            /*if (std::regex_match(lowerStopWord, re)) {
                return true;
            }*/
        }
        catch (const std::regex_error& e) {
            // Выводим сообщение об ошибке при работе с регулярными выражениями
            std::cerr << "Ошибка регулярного выражения: " << e.what() << std::endl;
        }

        // Если ни одно из условий выше не выполняется, возвращаем false
        return false;
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
    std::string input = "ОпреПрезидентамихотимчто-то мне сделать!";
    std::string output = JsonTree::toLower(input);
    std::cout << "Input: " << input << std::endl;
    std::cout << "Output: " << output << std::endl;

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