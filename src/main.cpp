#include "Application.h"
#include "ImGuiFrame.h"
#include "ScriptApi.h"
#include "ScriptFramework.h"
#include "ScriptHelper.h"
#include "ScriptInstance.h"

#include <filesystem>
#include <fstream>
#include <iostream>
#include <stdlib.h>
#include <string.h>

#pragma comment(lib, "Winmm.lib")
#pragma comment(lib, "Bcrypt.lib")
#pragma comment(lib, "version.lib")
#pragma comment(lib, "ws2_32")

static void ShowUpdateFrameworkLayout(bool* p_open, ScriptFramework scriptFramework)
{
    // Compile engine framework
    ImGui::SetNextWindowSize(ImVec2(500, 440), ImGuiCond_FirstUseEver);
    if (ImGui::Begin("Update Engine Framework", p_open))
    {
        static char inputDir[128] = "";
        ImGui::InputText("Framework Scripts Directory", inputDir, IM_ARRAYSIZE(inputDir));

        static char outputDir[128] = "";
        ImGui::InputText("Framework Output Directory", outputDir, IM_ARRAYSIZE(outputDir));

        if (ImGui::Button("UPDATE")) {
            mono_runtime_quit();
            scriptFramework.createFramework(inputDir, outputDir);
        }
    }
    ImGui::End();
}

static void ShowCompileScriptLayout(bool* p_open, ScriptFramework scriptFramework)
{
    ImGui::SetNextWindowSize(ImVec2(500, 440), ImGuiCond_FirstUseEver);

    if (ImGui::Begin("Compile Scripts", p_open))
    {
        static char inputDir[128] = "";
        ImGui::InputText("User Scripts Directory", inputDir, IM_ARRAYSIZE(inputDir));
        
        static char outputDir[128] = "";
        ImGui::InputText("Dll Output Directory", outputDir, IM_ARRAYSIZE(outputDir));
        
        static char frameworkDir[128] = "";
        ImGui::InputText("Engine Framework Directory", frameworkDir, IM_ARRAYSIZE(frameworkDir));

        if (ImGui::Button("COMPILE")) {
            mono_runtime_quit(); 
            // Compile if dll not found
            std::vector<std::string> userScripts;
            for (auto& script : scriptFramework.createDirVector(inputDir)) {
                if (script.find(".cs") != std::string::npos) {
                    std::string dllPath = outputDir + std::string("\\") + std::filesystem::path(script).stem().replace_extension(".dll").string();
                    if (!std::filesystem::exists(dllPath)) {
                        userScripts.push_back(script);
                    }
                }
            }
            scriptFramework.compileScripts(userScripts, frameworkDir);

            // Compile if engine is newer than dll
            std::vector<std::string> scriptsToCompile;
            for (auto& script : scriptFramework.createDirVector(outputDir)) {
                if (script.find(".dll") != std::string::npos) {
                    if (std::filesystem::path(script).stem() != std::string("Engine")) {
                        if (compareTimestamps(script, frameworkDir + std::string("\\Engine.dll"))) {
                            std::string scriptName = inputDir + std::string("\\") + std::filesystem::path(script).stem().replace_extension(".cs").string();
                            scriptsToCompile.push_back(scriptName);
                        }
                    }
                }
            }
            scriptFramework.compileScripts(scriptsToCompile, frameworkDir);

            // Compile if cs is newer than dll
            std::vector<std::string> updatedScripts;
            for (auto& script : scriptFramework.createDirVector(inputDir)) {
                if (script.find(".cs") != std::string::npos) {
                    if (compareTimestamps(outputDir + std::string("\\") + std::filesystem::path(script).stem().replace_extension(".dll").string(), script)) {
                        updatedScripts.push_back(script);
                    }
                }
            }
            scriptFramework.compileScripts(updatedScripts, frameworkDir);

        }
    }
    ImGui::End();
}

static void ShowScriptManagerLayout(bool* p_open, ScriptFramework scriptFramework, ScriptInstance* current)
{
    if (ImGui::Begin("Output Window")) {
        ImGui::Text("Script outputs");
    }
    ImGui::End();

    if (ImGui::Begin("Script Inspector")) {
        if (current)
        {
            for (size_t i = 0; i < current->getFieldCount(); ++i) {
                auto field = current->getField(i);
                ImGui::PushID(mono_field_get_name(field));
                if (mono_type_get_type(mono_field_get_type(field)) == MONO_TYPE_I4) {
                    int value = 0;
                    mono_field_get_value(current->object, field, &value);
                    if (ImGui::DragInt(mono_field_get_name(field), &value))
                        mono_field_set_value(current->object, field, &value);
                }

                else if (mono_type_get_type(mono_field_get_type(field)) == MONO_TYPE_STRING) {
                    MonoString* strval;
                    mono_field_get_value(current->object, field, &strval);
                    char* p = mono_string_to_utf8(strval);
                    int size = mono_string_length(strval);
                    if (ImGui::InputText(mono_field_get_name(field), p, size)) {
                        std::string data(p);
                        current->SetStringValue(scriptFramework.getDomain(), &data, mono_field_get_name(field));
                    }
                    mono_free(p);
                }
                else if (mono_type_get_type(mono_field_get_type(field)) == MONO_TYPE_R8) {
                    double value = 0;
                    mono_field_get_value(current->object, field, &value);
                    if (ImGui::DragScalarN(mono_field_get_name(field), ImGuiDataType_Double, (void*)&value, 1, 1))
                        mono_field_set_value(current->object, field, &value);
                }

                else if (mono_type_get_type(mono_field_get_type(field)) == MONO_TYPE_BOOLEAN) {
                    bool b = true;
                    mono_field_get_value(current->object, field, &b);
                    if (ImGui::Checkbox(mono_field_get_name(field), &b))
                        mono_field_set_value(current->object, field, &b);
                }
                ImGui::PopID();
            }
        }
    }
    ImGui::End();
}

int main(int arg, char* argv[])
{
    ScriptFramework scriptFramework;
    scriptFramework.initialize("data");
    Application app;
    scriptFramework.load("dlls");

    ImGuiFrame frame;
    frame.Initialize();
    
    std::vector<ScriptInstance> scriptInstances = scriptFramework.loadScripts(scriptFramework.createDirVector("dlls"));
    for (auto& script : scriptInstances) {
        script.deserializeData(scriptFramework.getDomain(), "userScripts");
        script.init();
    }

    bool load_user_scripts = false;
    bool update_engine_framework = false;
    bool script_manager = true;

    bool done = false;
    while (!done) {
        MSG msg;
        while (::PeekMessage(&msg, NULL, 0U, 0U, PM_REMOVE)) {
            ::TranslateMessage(&msg);
            ::DispatchMessage(&msg);
            if (msg.message == WM_QUIT)
                done = true;
        }
        if (done)
            break;

        frame.CreateFrame();

        static bool simulationRunning = false;
        static int selected = -1;

        ScriptInstance* current = nullptr;
        if (selected != -1 && selected < scriptInstances.size())
            current = &scriptInstances.at(selected);


        if (simulationRunning) {
            current->update();
        }

        if (ImGui::Begin("Script Manager")) {
            // For every script loaded
            for (int i = 0; i < scriptInstances.size(); ++i) {
                if (ImGui::Selectable(scriptInstances.at(i).getClassName(), i == selected))
                    selected = i;
            }
        }
        ImGui::End();

        {
            if (ImGui::BeginMainMenuBar()) {
                if (ImGui::BeginMenu("File")) {
                    if (ImGui::MenuItem("Update Engine Framework")) {
                        update_engine_framework = true;
                    }
                    if (ImGui::MenuItem("Compile User Scripts")) {
                        load_user_scripts = true;
                    }
                    if (ImGui::MenuItem("Script Manager")) {
                        script_manager = true;
                    }
                    ImGui::EndMenu();
                }

                if (ImGui::BeginMenu("Simulation")) {
                    if (!simulationRunning) {
                        if (ImGui::MenuItem("Start"))
                            simulationRunning = true;
                    } else {
                        if (ImGui::MenuItem("Stop"))
                            simulationRunning = false;
                    }
                    ImGui::EndMenu();
                }

                ImGui::EndMainMenuBar();
            }

            if (update_engine_framework) 
                ShowUpdateFrameworkLayout(&update_engine_framework, scriptFramework);

            if (load_user_scripts) 
                ShowCompileScriptLayout(&load_user_scripts, scriptFramework);

            if (script_manager)
                ShowScriptManagerLayout(&script_manager, scriptFramework, current);
        }
        frame.Render();
    }
    frame.EndFrame();
  
    return 0;
}
