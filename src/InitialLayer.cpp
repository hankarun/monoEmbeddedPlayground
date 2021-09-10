#include "InitialLayer.h"
#include "../imgui/imgui.h"

#include "ScriptCompiler.h"

void InitialLayer::load()
{
    // Load mono domain
}

void InitialLayer::draw()
{
    ImGui::Begin("Intializing");

    if (ImGui::Button("Compile Framework"))
    {
        ScriptCompiler compiler("./data/mono/roslyn/csc.exe");
        compiler.compile({ "scripts/Engine.cs", "scripts/Debug.cs", "scripts/Time.cs" }, "engine.dll", {});
    }

    ImGui::End();
}

void InitialLayer::destroy()
{
}
