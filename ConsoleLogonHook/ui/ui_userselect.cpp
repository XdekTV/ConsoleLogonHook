#include "ui_userselect.h"
#include "detours.h"
#include "spdlog/spdlog.h"
#include "../util/util.h"
#include <winstring.h>
#include "ui_helper.h"
#include <vector>

std::vector<SelectableUserOrCredentialControlWrapper> buttons;
const int signInOptionChoice = 0;
int choiceIteration = 0;
bool wasInSelectedCredentialView = false;

__int64(__fastcall* CredProvSelectionView__v_OnKeyInput)(void* _this, const struct _KEY_EVENT_RECORD* a2, int* a3);
__int64(__fastcall* UserSelectionView__v_OnKeyInput)(void* _this, const struct _KEY_EVENT_RECORD* a2, int* a3);


//This will need to be moved for when selectected credential view is made, and a global or prop will need to be made so that the bool can be saved
__int64(__fastcall* SelectedCredentialView__v_OnKeyInput)(void* _this, const struct _KEY_EVENT_RECORD* a2, int* a3);
__int64 SelectedCredentialView__v_OnKeyInput_Hook(void* _this, const struct _KEY_EVENT_RECORD* a2, int* a3)
{
    if (a2->wVirtualKeyCode == VK_ESCAPE || a2->wVirtualKeyCode == VK_BACK)
    {
        wasInSelectedCredentialView = true;
    }

    SPDLOG_INFO("SelectedCredentialView__v_OnKeyInput_Hook");

    return SelectedCredentialView__v_OnKeyInput(_this,a2,a3);
}

void* UserSelectionView = 0;

__int64(__fastcall* UserSelectionView__RuntimeClassInitialize)(void* _this, void* a2);
__int64 UserSelectionView__RuntimeClassInitialize_Hook(void* _this, void* a2)
{
    SPDLOG_INFO("UserSelectionView__RuntimeClassInitialize_Hook a2 [{}]", a2);
    UserSelectionView = _this;

    auto userSelect = uiRenderer::Get()->GetWindowOfTypeId<uiUserSelect>(5);
    if (userSelect)
    {
        SPDLOG_INFO("Setting active status userSelect");
        userSelect->SetActive();
    }

    return UserSelectionView__RuntimeClassInitialize(_this, a2);
}

void* CredProvSelectionView;

__int64(__fastcall* CredProvSelectionView__RuntimeClassInitialize)(void* _this, void* a2, HSTRING a3, char a4);
__int64 CredProvSelectionView__RuntimeClassInitialize_Hook(void* _this, void* a2, HSTRING a3, char a4)
{
    SPDLOG_INFO("CredProvSelectionView__RuntimeClassInitialize a3 [{}]", ws2s(ConvertHStringToString(a3)));
    choiceIteration = 0;
    CredProvSelectionView = _this;
    return CredProvSelectionView__RuntimeClassInitialize(_this,a2,a3,a4);
}

__int64 (__fastcall* SelectableUserOrCredentialControl__RuntimeClassInitialize)(void* _this, void* a2, void* a3);
__int64 SelectableUserOrCredentialControl__RuntimeClassInitialize_Hook(void* _this, void* a2, void* a3)
{
    auto res = SelectableUserOrCredentialControl__RuntimeClassInitialize(_this,a2,a3);

    SelectableUserOrCredentialControlWrapper wrapper;
    wrapper.actualInstance = _this;

    if (wrapper.isCredentialControl())
    {
        if (choiceIteration == signInOptionChoice)
        {
            if (wasInSelectedCredentialView)
            {
                wrapper.virtualKeyCodeToPress = VK_ESCAPE;
                wasInSelectedCredentialView = false;
            }
            wrapper.markedPressed = true; // it wont work if we press here
        }
        choiceIteration++;
    }

    SPDLOG_INFO("SelectableUserOrCredentialControl__RuntimeClassInitialize_Hook, user name {} this {} a3 {}", ws2s(wrapper.GetText()),_this,a3);
    buttons.push_back(wrapper);

    return res;
}

void* ConsoleUIView;

__int64(__fastcall*ConsoleUIView__Initialize)(void* _this);
__int64 ConsoleUIView__Initialize_Hook(void* _this)
{
    ConsoleUIView = _this;
    return ConsoleUIView__Initialize(_this);
}

void* (__fastcall* SelectableUserOrCredentialControl_Destructor)(void* _this,char a2);
void* SelectableUserOrCredentialControl_Destructor_Hook(void* _this, char a2)
{
    for (int i = 0; i < buttons.size(); ++i)
    {
        auto& button = buttons[i];
        if (button.actualInstance == _this)
        {
            if (wasInSelectedCredentialView)
            {
                button.virtualKeyCodeToPress = VK_ESCAPE;
                wasInSelectedCredentialView = false;
            }

            SPDLOG_INFO("Found button instance and removing!");
            buttons.erase(buttons.begin() + i);
            break;
        }
    }


    return SelectableUserOrCredentialControl_Destructor(_this,a2);
}

void uiUserSelect::Tick()
{
    for (int i = 0; i < buttons.size(); ++i)
    {
        auto& button = buttons[i];
        if (button.markedPressed)
        {
            button.Press();
            button.markedPressed = false;
        }
    }
}

void uiUserSelect::Draw()
{
    ImGui::Begin("Status View", 0, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoBackground);

    ImGui::SetWindowSize(ImGui::GetIO().DisplaySize);
    ImGui::SetWindowPos(ImVec2(0, 0));

    ImGui::SetCursorPosY(ImGui::GetCursorPosY() + ImGui::GetContentRegionAvail().y * 0.5);
    ImGui::SetCursorPosX(ImGui::GetCursorPosX() + ImGui::GetContentRegionAvail().x * 0.5);

    ImGui::BeginChild("", ImVec2(ImGui::GetContentRegionAvail().x * 0.5, ImGui::GetContentRegionAvail().y * 0.5),0,ImGuiWindowFlags_HorizontalScrollbar);
    for (int i = 0; i < buttons.size(); ++i)
    {
        auto& button = buttons[i];
        if (!button.actualInstance)
            continue;

        if (button.isCredentialControl())
            continue;

        if (ImGui::Button(ws2s(button.GetText()).c_str()))
            button.Press();

        if (i != buttons.size() - 1)
            ImGui::SameLine();
    }
    ImGui::EndChild();

    ImGui::End();
}

void uiUserSelect::InitHooks(uintptr_t baseaddress)
{
    UserSelectionView__RuntimeClassInitialize = decltype(UserSelectionView__RuntimeClassInitialize)(baseaddress + 0x378F0);
    SelectableUserOrCredentialControl__RuntimeClassInitialize = decltype(SelectableUserOrCredentialControl__RuntimeClassInitialize)(baseaddress + 0x3FD24);
    CredProvSelectionView__RuntimeClassInitialize = decltype(CredProvSelectionView__RuntimeClassInitialize)(baseaddress + 0x35640);
    CredProvSelectionView__v_OnKeyInput = decltype(CredProvSelectionView__v_OnKeyInput)(baseaddress + 0x35A00);
    SelectedCredentialView__v_OnKeyInput = decltype(SelectedCredentialView__v_OnKeyInput)(baseaddress + 0x38720);
    SelectableUserOrCredentialControl_Destructor = decltype(SelectableUserOrCredentialControl_Destructor)(baseaddress + 0x363A8);
    UserSelectionView__v_OnKeyInput = decltype(UserSelectionView__v_OnKeyInput)(baseaddress + 0x37C30);

    ConsoleUIView__Initialize = decltype(ConsoleUIView__Initialize)(baseaddress + 0x42710);
    //ConsoleUIView__HandleKeyInput = decltype(ConsoleUIView__HandleKeyInput)(baseaddress + 0x43530);

    Hook(UserSelectionView__RuntimeClassInitialize, UserSelectionView__RuntimeClassInitialize_Hook);
    Hook(SelectableUserOrCredentialControl__RuntimeClassInitialize, SelectableUserOrCredentialControl__RuntimeClassInitialize_Hook);
    Hook(CredProvSelectionView__RuntimeClassInitialize, CredProvSelectionView__RuntimeClassInitialize_Hook);
    Hook(SelectedCredentialView__v_OnKeyInput, SelectedCredentialView__v_OnKeyInput_Hook);
    Hook(SelectableUserOrCredentialControl_Destructor, SelectableUserOrCredentialControl_Destructor_Hook);
    Hook(ConsoleUIView__Initialize, ConsoleUIView__Initialize_Hook);
}

std::wstring SelectableUserOrCredentialControlWrapper::GetText()
{
    if (hastext)
        return text;

    uintptr_t pointer = *(uintptr_t*)(__int64(actualInstance) + 0x58);
    if (!pointer)
        pointer = *(uintptr_t*)(__int64(actualInstance) + 0x50);

    HSTRING hstring;

    __int64 result = (*(__int64(__fastcall**)(__int64, HSTRING*))(*(uintptr_t*)pointer + 0x40i64))(pointer, &hstring); //keep this line as is, this is very funky when it comes down to specifics
    if (result < 0)
        return L""; //failure

    text = ConvertHStringToString(hstring);
    hastext = true;

    fWindowsDeleteString(hstring);
    return text;
}

void SelectableUserOrCredentialControlWrapper::Press()
{
    _KEY_EVENT_RECORD keyrecord;
    keyrecord.bKeyDown = true;
    keyrecord.wVirtualKeyCode = virtualKeyCodeToPress;
    int result;
    if (actualInstance)
    {
        SPDLOG_INFO("Actual instance {} {} isn't null, so we are calling handlekeyinput with enter on the control!", actualInstance, virtualKeyCodeToPress);

        void* instanceToUse = nullptr;

        if (isCredentialControl())
        {
            if (!CredProvSelectionView)
            {
                SPDLOG_INFO("CredProvSelectionView IS NULL< CANNOT PRESS!");

            }
            instanceToUse = CredProvSelectionView;
        }
        else
        {
            if (!UserSelectionView)
            {
                SPDLOG_INFO("UserSelectionView IS NULL< CANNOT PRESS!");

            }
            instanceToUse = CredProvSelectionView;
        }

        if (instanceToUse)
        {
            *(void**)(__int64(instanceToUse) + 0x70) = actualInstance;
            if (isCredentialControl())
                CredProvSelectionView__v_OnKeyInput((void*)(__int64(instanceToUse) + 8),&keyrecord,&result);
            else
            {
                SPDLOG_INFO("Calling UserSelectionView__v_OnKeyInput");
                *(void**)(__int64(instanceToUse) + 0x70) = actualInstance;
                UserSelectionView__v_OnKeyInput((void*)(__int64(instanceToUse) + 8), &keyrecord, &result);
            }

            virtualKeyCodeToPress = VK_RETURN; //reset after an override
        }
        //else
        //{
        //    *(void**)(__int64(UserSelectionView) + 0x70) = actualInstance;
        //    UserSelectionView__v_OnKeyInput((void*)(__int64(UserSelectionView) + 8), &keyrecord, &result);
        //}
    }
}

bool SelectableUserOrCredentialControlWrapper::isCredentialControl()
{
    return *(uintptr_t*)(__int64(actualInstance) + 0x50) != NULL;
}
