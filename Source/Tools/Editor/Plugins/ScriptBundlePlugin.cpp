//
// Copyright (c) 2017-2019 the rbfx project.
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

#include <Urho3D/Resource/ResourceEvents.h>
#include <Urho3D/Script/Script.h>
#include "Plugins/ScriptBundlePlugin.h"

#if URHO3D_PLUGINS && URHO3D_CSHARP
namespace Urho3D
{

ScriptBundlePlugin::ScriptBundlePlugin(Context* context)
    : Plugin(context)
{
    SubscribeToEvent(E_FILECHANGED, [this](StringHash, VariantMap& args) { OnFileChanged(args); });
}

bool ScriptBundlePlugin::Load()
{
    application_ = Script::GetRuntimeApi()->CompileResourceScriptPlugin();
    if (application_.NotNull())
    {
        application_->InitializeReloadablePlugin();
        unloading_ = false;
        outOfDate_ = false;
        version_++;
        return true;
    }
    return false;
}

bool ScriptBundlePlugin::PerformUnload()
{
    application_->UninitializeReloadablePlugin();
    Script::GetRuntimeApi()->Dispose(application_.Detach());
    return true;
}

void ScriptBundlePlugin::OnFileChanged(VariantMap& args)
{
    using namespace FileChanged;
    const ea::string& name = args[P_RESOURCENAME].GetString();
    outOfDate_ |= name.ends_with(".cs");
    if (outOfDate_ && !IsLoaded())
    {
        Load();
        outOfDate_ = false;
    }
}

}
#endif