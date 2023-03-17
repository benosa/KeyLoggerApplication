#pragma once
#include <string>
#include "IGuardProcessor.h"

class NullGuardProcessor :
    public IGuardProcessor
{
public:
    std::string process(std::string stopWord, std::wstring sequence);
};

