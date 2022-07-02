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

#include "../Foundation/ModelImporter.h"

#include <Urho3D/IO/FileSystem.h>
#include <Urho3D/Utility/GLTFImporter.h>

namespace Urho3D
{

namespace
{

bool IsFileNameGLTF(const ea::string& fileName)
{
    return fileName.ends_with(".gltf")
        || fileName.ends_with(".glb");
}

}

void Foundation_ModelImporter(Context* context, ProjectEditor* project)
{
    if (!context->IsReflected<ModelImporter>())
        context->RegisterFactory<ModelImporter>();
}

ModelImporter::ModelImporter(Context* context)
    : AssetTransformer(context)
{
}

bool ModelImporter::IsApplicable(const AssetTransformerInput& input)
{
    return IsFileNameGLTF(input.resourceName_);
}

bool ModelImporter::Execute(const AssetTransformerInput& input, AssetTransformerOutput& output)
{
    const GLTFImporterSettings importerSettings;
    auto importer = MakeShared<GLTFImporter>(context_, importerSettings);

    if (!importer->LoadFile(input.inputFileName_, AddTrailingSlash(input.outputFileName_), AddTrailingSlash(input.resourceName_)))
    {
        URHO3D_LOGERROR("Failed to load asset {} as GLTF model", input.resourceName_);
        return false;
    }

    auto fs = GetSubsystem<FileSystem>();
    fs->RemoveDir(input.outputFileName_, true);
    fs->Delete(input.outputFileName_);

    if (!importer->SaveResources())
    {
        URHO3D_LOGERROR("Failed to save output files for asset {}", input.resourceName_);
        return false;
    }

    return true;
}

}