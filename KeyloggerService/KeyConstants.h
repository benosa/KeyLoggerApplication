#pragma once
#ifndef KEYCONSTANTS_H
#define KEYCONSTANTS_H

#include <map>
#include <string>
#include <Windows.h>
#include <iostream>
#include <string>
#include <map>

//class KeyPair
//{
//public:
//	KeyPair(const std::wstring& vk = L"", const std::wstring& hiName = L"", const std::wstring& lowName = L"") : VKName(vk), hiName(hiName), lowName(lowName){}
//
//	std::wstring VKName;
//	std::wstring lowName;
//	std::wstring hiName;
//};

class Keys // store map of keys we get from the system and translate them to human friendly values
{
public:
	//static std::map<int, KeyPair> ENG_KEYS;
	//static std::map<int, KeyPair> RUS_KEYS;
	static std::wstring key(int lang, int key, bool shift, bool capital);
};

#endif // KEYCONSTANTS_H