#pragma once
#include <sstream>
#include <unicode/regex.h>
#include <unicode/unistr.h>

class IGuardProcessor {
public:
	virtual std::string process(std::string stopWord, std::wstring sequence) = 0;
};