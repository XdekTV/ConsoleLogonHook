#pragma once
#include "ui_main.h"

struct SecurityOptionControlWrapper
{
    void* actualInstance;

    SecurityOptionControlWrapper(void* instance)
    {
        actualInstance = instance;
    }

    wchar_t* getString()
    {
        return *(wchar_t**)(__int64(actualInstance) + 0x48);
    }

    void Press();
};

class uiSecurityControl : public uiWindow
{
public:

    uiSecurityControl()
    {
        windowTypeId = 2;
    }

    void Draw() override;

    static void InitHooks(uintptr_t baseaddress);
};