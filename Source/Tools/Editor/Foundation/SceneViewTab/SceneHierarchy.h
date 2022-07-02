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

#pragma once

#include "../../Foundation/Shared/HierarchyBrowserSource.h"
#include "../../Foundation/SceneViewTab.h"

namespace Urho3D
{

void Foundation_SceneHierarchy(Context* context, SceneViewTab* sceneViewTab);

/// Scene hierarchy provider for hierarchy browser tab.
class SceneHierarchy : public SceneViewAddon, public HierarchyBrowserSource
{
    URHO3D_OBJECT(SceneHierarchy, SceneViewAddon)

public:
    explicit SceneHierarchy(SceneViewTab* sceneViewTab);

    /// Implement SceneViewAddon
    /// @{
    ea::string GetUniqueName() const override { return "SceneHierarchy"; }
    void WriteIniSettings(ImGuiTextBuffer& output) override;
    void ReadIniSettings(const char* line) override;
    /// @}

    /// Implement HierarchyBrowserSource
    /// @{
    EditorTab* GetOwnerTab() override { return owner_; }

    void RenderContent() override;
    void RenderContextMenuItems() override;
    void RenderMenu() override;
    void ApplyHotkeys(HotkeyManager* hotkeyManager) override;
    /// @}

private:
    void RenderToolbar(SceneViewPage& page);

    void RenderNode(SceneViewPage& page, Node* node);
    void RenderComponent(SceneViewPage& page, Component* component);
    void ProcessObjectSelected(SceneViewPage& page, Object* object, bool toggle, bool range);
    void UpdateActiveObjectVisibility(SceneViewPage& page, Object* currentObject);

    void BeginRangeSelection();
    void ProcessRangeSelection(Object* currentObject, bool open);
    void EndRangeSelection(SceneViewPage& page);

    void UpdateSearchResults(SceneViewPage& page);

    struct RangeSelectionRequest
    {
        WeakPtr<Object> from_;
        WeakPtr<Object> to_;

        bool IsBorder(Object* object) const { return object == from_ || object == to_; }
    };

    WeakPtr<SceneViewTab> owner_;
    bool reentrant_{};

    bool showTemporary_{};
    bool showComponents_{true};

    /// UI state
    /// @{
    bool isActiveObjectVisible_{};
    bool wasActiveObjectVisible_{};

    struct RangeSelection
    {
        ea::optional<RangeSelectionRequest> pendingRequest_;

        ea::optional<RangeSelectionRequest> currentRequest_;
        bool isActive_{};
        ea::vector<WeakPtr<Object>> result_;
    } rangeSelection_;

    struct NodeSearch
    {
        ea::string currentQuery_;
        WeakPtr<Scene> lastScene_;
        ea::string lastQuery_;
        ea::vector<WeakPtr<Node>> lastResults_;
    } search_;
    /// @}
};

}