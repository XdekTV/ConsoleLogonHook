#include "ui_statusview.h"
#include <Windows.h>
#include "detours.h"
#include "spdlog/spdlog.h"
#include "../util/util.h"
#include <winstring.h>
#include <util/interop.h>

__int64(__fastcall* StatusView__RuntimeClassInitialize)(/*StatusView*/void* _this, HSTRING a2, /*IUser*/void* a3);
__int64 StatusView__RuntimeClassInitialize_Hook(/*StatusView*/void* _this, HSTRING a2, /*IUser*/void* a3)
{
    auto convertString = [&](HSTRING str) -> std::wstring
        {
            const wchar_t* convertedString = fWindowsGetStringRawBuffer(a2, 0);
            return convertedString;
        };

    std::wstring text = convertString(a2);

    external::StatusView_SetActive(text);

    //auto statusview = uiRenderer::Get()->GetWindowOfTypeId<uiStatusView>(4);
    //if (statusview)
    //{
    //    SPDLOG_INFO("Setting active status view instance");
    //    statusview->statusInstance = _this;
    //    statusview->statusText = text;
    //    statusview->SetActive();
    //}
    //
    //MinimizeLogonConsole();
    /*auto stringtobytes = [&](std::wstring str) -> std::string
        {
            std::stringstream ss;
            ss << std::hex;

            for (int i = 0; i < str.length(); ++i)
            {
                wchar_t Short = str.at(i);
                char upperbyte = HIBYTE(Short);
                char lowerbyte = LOBYTE(Short);

                ss << std::setw(2) << std::setfill('0') << (int)upperbyte << " ";
                ss << std::setw(2) << std::setfill('0') << (int)lowerbyte << " ";

            }
            return ss.str();
        };*/

    SPDLOG_INFO("StatusView__RuntimeClassInitialize a1[{}] a2[{}] a3[{}]", (void*)_this, ws2s(text).c_str(), a3);

    return StatusView__RuntimeClassInitialize(_this, a2, a3);
}

void* (__fastcall* StatusView__Destructor)(/*StatusView*/void* _this, char a2);
void* StatusView__Destructor_Hook(void* _this, char a2)
{
    //this has issues during windows update shit, works fine elsewhere
    /*auto statusview = uiRenderer::Get()->GetWindowOfTypeId<uiStatusView>(4);
    if (statusview)
    {
        SPDLOG_INFO("setting inactive status view instance");
        statusview->statusInstance = 0;
        statusview->statusText = L"";
        statusview->SetInactive();
    }*/
    external::MessageOrStatusView_Destroy();

    return StatusView__Destructor(_this,a2);
}

void uiStatusView::InitHooks(uintptr_t baseaddress)
{
    StatusView__RuntimeClassInitialize = decltype(StatusView__RuntimeClassInitialize)(baseaddress + 0x387D0);
    StatusView__Destructor = decltype(StatusView__Destructor)(baseaddress + 0x224F8);

    Hook(StatusView__RuntimeClassInitialize, StatusView__RuntimeClassInitialize_Hook);
    Hook(StatusView__Destructor, StatusView__Destructor_Hook);
}
