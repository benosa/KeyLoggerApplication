#pragma once
#include <string>
#include "IGuardProcessor.h"

class NullGuardProcessor :
    public IGuardProcessor
{
public:
    bool process(bool stopStatus, std::wstring sequence);
};

