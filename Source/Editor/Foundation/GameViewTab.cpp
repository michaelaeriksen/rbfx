//
// Copyright (c) 2017-2020 the rbfx project.
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
// THE SOFTWARE.
//

#include "../Foundation/GameViewTab.h"

#include "../Core/IniHelpers.h"

#include <Urho3D/Core/WorkQueue.h>
#include <Urho3D/Engine/StateManager.h>
#include <Urho3D/Graphics/Graphics.h>
#include <Urho3D/Graphics/GraphicsEvents.h>
#include <Urho3D/Graphics/Renderer.h>
#include <Urho3D/Graphics/Texture2D.h>
#include <Urho3D/Input/Input.h>
#include <Urho3D/Plugins/PluginManager.h>
#include <Urho3D/RenderAPI/RenderContext.h>
#include <Urho3D/RenderAPI/RenderDevice.h>
#include <Urho3D/Resource/ResourceCache.h>
#include <Urho3D/Resource/XMLFile.h>
#include <Urho3D/Scene/Scene.h>
#include <Urho3D/SystemUI/DebugHud.h>
#include <Urho3D/SystemUI/Widgets.h>
#include <Urho3D/UI/UI.h>

#if URHO3D_RMLUI
    #include <Urho3D/RmlUI/RmlUI.h>
#endif

#include <IconFontCppHeaders/IconsFontAwesome6.h>

namespace Urho3D
{

namespace
{

const auto Hotkey_ReleaseInput = EditorHotkey{"GameViewTab.ReleaseInput"}.Shift().Press(KEY_ESCAPE);

}

void Foundation_GameViewTab(Context* context, Project* project)
{
    project->AddTab(MakeShared<GameViewTab>(context));
}

class GameViewTab::PlayState : public Object
{
    URHO3D_OBJECT(PlayState, Object);

public:
    PlayState(Context* context, CustomBackbufferTexture* backbuffer)
        : Object(context)
        , renderer_(GetSubsystem<Renderer>())
        , pluginManager_(GetSubsystem<PluginManager>())
        , input_(GetSubsystem<Input>())
        , legacyUI_(GetSubsystem<UI>())
        , systemUI_(GetSubsystem<SystemUI>())
#if URHO3D_RMLUI
        , rmlUI_(GetSubsystem<RmlUI>())
#endif
        , stateManager_(GetSubsystem<StateManager>())
        , project_(GetSubsystem<Project>())
        , backbuffer_(backbuffer)
    {
        UpdateRenderSurface();

        legacyUI_->GetRoot()->RemoveAllChildren();
        legacyUI_->GetRootModalElement()->RemoveAllChildren();

        legacyUI_->SetRenderTarget(backbuffer_->GetTexture());
        backbuffer_->SetActive(true);
        GrabInput();

        pluginManager_->StartApplication();
        UpdatePreferredMouseSetup();

        SubscribeToEvent(E_BEGINRENDERING, [this]
        {
            auto renderDevice = GetSubsystem<RenderDevice>();
            RenderContext* renderContext = renderDevice->GetRenderContext();

            RenderTargetView renderTargets[] = {RenderTargetView::Texture(backbuffer_->GetTexture())};
            renderContext->SetRenderTargets(ea::nullopt, renderTargets);
            renderContext->ClearRenderTarget(0, 0x245953_rgb);

            UnsubscribeFromEvent(E_BEGINRENDERING);
        });
    }

    void GrabInput()
    {
        if (inputGrabbed_)
            return;

        input_->SetMouseVisible(preferredMouseVisible_);
        input_->SetMouseMode(preferredMouseMode_);
        input_->SetEnabled(true);
        systemUI_->SetPassThroughEvents(true);
        project_->SetGlobalHotkeysEnabled(false);
        project_->SetHighlightEnabled(true);

        inputGrabbed_ = true;
    }

    void ReleaseInput()
    {
        if (!inputGrabbed_)
            return;

        UpdatePreferredMouseSetup();
        input_->SetMouseVisible(true);
        input_->SetMouseMode(MM_ABSOLUTE);
        input_->SetEnabled(false);
        systemUI_->SetPassThroughEvents(false);
        project_->SetGlobalHotkeysEnabled(true);
        project_->SetHighlightEnabled(false);

        inputGrabbed_ = false;
    }

    void Update(const IntRect& windowRect)
    {
        input_->SetExplicitWindowRect(windowRect);
        UpdateRenderSurface();
    }

    bool IsInputGrabbed() const { return inputGrabbed_; }

    ~PlayState()
    {
        ReleaseInput();

        pluginManager_->StopApplication();
        backbuffer_->SetActive(false);

        legacyUI_->SetRenderTarget(nullptr);
        legacyUI_->SetCustomSize({0, 0});
        legacyUI_->GetRoot()->RemoveAllChildren();
        legacyUI_->GetRootModalElement()->RemoveAllChildren();

#if URHO3D_RMLUI
        rmlUI_->SetRenderTarget(nullptr);
#endif

        input_->ResetExplicitWindowRect();

        renderer_->SetBackbufferRenderSurface(nullptr);
        renderer_->SetNumViewports(0);

        stateManager_->Reset();
    }

private:
    void UpdatePreferredMouseSetup()
    {
        preferredMouseVisible_ = input_->IsMouseVisible();
        preferredMouseMode_ = input_->GetMouseMode();
    }

    void UpdateRenderSurface()
    {
        RenderSurface* backbufferSurface = backbuffer_->GetTexture()->GetRenderSurface();
        if (backbufferSurface_ != backbufferSurface)
        {
            backbufferSurface_ = backbufferSurface;
            renderer_->SetBackbufferRenderSurface(backbufferSurface_);
            legacyUI_->SetCustomSize(backbufferSurface->GetSize());
#if URHO3D_RMLUI
            rmlUI_->SetRenderTarget(backbufferSurface_);
#endif
        }
    }

    Renderer* renderer_{};
    PluginManager* pluginManager_{};
    Input* input_{};
    UI* legacyUI_{};
    SystemUI* systemUI_{};
#if URHO3D_RMLUI
    RmlUI* rmlUI_{};
#endif
    StateManager* stateManager_{};
    Project* project_{};

    CustomBackbufferTexture* backbuffer_{};
    WeakPtr<RenderSurface> backbufferSurface_;

    bool inputGrabbed_{};

    bool preferredMouseVisible_{true};
    MouseMode preferredMouseMode_{MM_FREE};

    IntVector2 oldRootElementSize_{};
};

GameViewTab::GameViewTab(Context* context)
    : EditorTab(context, "Game", "212a6577-8a2a-42d6-aaed-042d226c724c",
          EditorTabFlag::NoContentPadding | EditorTabFlag::OpenByDefault, EditorTabPlacement::DockCenter)
    , backbuffer_(MakeShared<CustomBackbufferTexture>(context_))
{
    BindHotkey(Hotkey_ReleaseInput, &GameViewTab::ReleaseInput);

    auto pluginManager = GetSubsystem<PluginManager>();
    pluginManager->SetQuitApplicationCallback([this] { QuitApplication(); });
}

GameViewTab::~GameViewTab()
{
    auto pluginManager = GetSubsystem<PluginManager>();
    pluginManager->SetQuitApplicationCallback([] {});
}

bool GameViewTab::IsInputGrabbed() const
{
    return state_ && state_->IsInputGrabbed();
}

void GameViewTab::Play()
{
    if (state_)
        Stop();

    state_ = ea::make_unique<PlayState>(context_, backbuffer_);
    OnSimulationStarted(this);
}

void GameViewTab::Stop()
{
    if (!state_)
        return;

    state_ = nullptr;
    OnSimulationStopped(this);
}

void GameViewTab::TogglePlayed()
{
    if (IsPlaying())
        Stop();
    else
        Play();
}

void GameViewTab::ReleaseInput()
{
    if (state_)
        state_->ReleaseInput();
}

void GameViewTab::RenderToolbar()
{
    if (Widgets::ToolbarButton(ICON_FA_BUG, "Toggle Debug HUD", hudVisible_))
        hudVisible_ = !hudVisible_;

    Widgets::ToolbarSeparator();
}

void GameViewTab::RenderContent()
{
    const IntVector2 contentSize = GetContentSize();
    if (contentSize.x_ == 0 || contentSize.y_ == 0)
        return;

    backbuffer_->SetTextureSize(contentSize);
    backbuffer_->Update();

    if (state_)
    {
        Texture2D* sceneTexture = backbuffer_->GetTexture();
        Widgets::ImageItem(sceneTexture, ToImGui(sceneTexture->GetSize()));

#if URHO3D_SYSTEMUI_VIEWPORTS
        const IntVector2 origin = IntVector2::ZERO;
#else
        auto graphics = GetSubsystem<Graphics>();
        const IntVector2 origin = graphics->GetWindowPosition();
#endif
        const IntVector2 windowMin = origin + ToIntVector2(ui::GetItemRectMin());
        const IntVector2 windowMax = origin + ToIntVector2(ui::GetItemRectMax());
        state_->Update({windowMin, windowMax});
    }

    if (state_)
    {
        const bool wasGrabbed = state_->IsInputGrabbed();
        const bool needGrab = ui::IsItemHovered() && ui::IsMouseClicked(MOUSEB_ANY);
        const bool needRelease = !ui::IsItemHovered() && ui::IsMouseClicked(MOUSEB_ANY);
        if (!wasGrabbed && needGrab)
            state_->GrabInput();
        else if (wasGrabbed && needRelease)
            state_->ReleaseInput();
    }

    auto hud = GetSubsystem<DebugHud>();
    if (hud && hudVisible_)
    {
        const IntVector2 position = GetContentPosition();
        ui::SetCursorScreenPos(ToImGui(position.ToVector2()));
        hud->RenderUI(DEBUGHUD_SHOW_ALL);
    }
}

void GameViewTab::RenderContextMenuItems()
{
}

void GameViewTab::WriteIniSettings(ImGuiTextBuffer& output)
{
    WriteIntToIni(output, "IsHudVisible", hudVisible_ ? 1 : 0);
}

void GameViewTab::ReadIniSettings(const char* line)
{
    if (const auto isHudVisible = ReadIntFromIni(line, "IsHudVisible"))
        hudVisible_ = *isHudVisible != 0;
}

void GameViewTab::QuitApplication()
{
    auto workQueue = GetSubsystem<WorkQueue>();
    if (IsPlaying())
        workQueue->PostDelayedTaskForMainThread([this] { Stop(); });
    else
        URHO3D_LOGWARNING("Application quit was requested when nothing is played");
}

} // namespace Urho3D
