#pragma once
#include <gmock/gmock.h>
#include "../KeyloggerService/IGuardProcessor.h"

class GuardProcessorMock : public IGuardProcessor
{
public:
    MOCK_METHOD2(process, bool(bool stopStatus, std::wstring sequence));
};