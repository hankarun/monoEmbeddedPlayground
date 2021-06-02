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

void ShowScriptManagerLayout(bool* p_open, ScriptFramework scriptFramework, ScriptInstance* current)
{
    if (ImGui::Begin("Output Window")) {
        ImGui::TextUnformatted(getStream().str().c_str());
    }
    ImGui::End();

    if (ImGui::Begin("Script Inspector")) {
        if (current) {
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
                    static std::string buffer;
                    buffer = p;
                    buffer.resize(size + 10.0);
                    if (ImGui::InputText(mono_field_get_name(field), buffer.data(), buffer.size())) {
                        current->SetStringValue(scriptFramework.getDomain(), &buffer, mono_field_get_name(field));
                    }
                    mono_free(p);
                } else if (mono_type_get_type(mono_field_get_type(field)) == MONO_TYPE_R8) {
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

            ImGui::Text("Script Functions");
            for (auto& k : current->methods)
            {
                if (ImGui::Button(k.first.c_str()))
                {
                    current->runMethod(k.first.c_str());
                }
            }
        }
    }
    ImGui::End();
}

int main(int arg, char* argv[])
{
    const char* dataDir = "dlls";
    const char* engineDir = "scripts";
    const char* monoDir = "data";

    ScriptFramework scriptFramework;
    scriptFramework.initialize(monoDir);
    Application app;
    if (!std::filesystem::exists("dlls/Engine.dll"))
    {
        printf("Engine dll not found, compiling.\n");
        scriptFramework.createFramework(engineDir, dataDir);
    }
    scriptFramework.load(dataDir);

    ImGuiFrame frame;
    frame.Initialize();

    std::vector<ScriptInstance> scriptInstances;

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
            if (current)
                current->runMethod("Update");
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
                    if (ImGui::MenuItem("Load scripts from userscripts directory")) {

                        for (auto& script : scriptInstances) {
                            script.unload(scriptFramework.getDomain());
                        }
                        scriptInstances.clear();

                        const char* inputDir = "userScripts";
                        std::vector<std::string> userScripts;
                        std::vector<std::string> scriptsToLoad;
                        for (auto& script : scriptFramework.createDirVector(inputDir)) {
                            if (script.find(".cs") != std::string::npos) {
                                std::string dllPath = dataDir + std::string("\\") + std::filesystem::path(script).stem().replace_extension(".dll").string();
                                if (!std::filesystem::exists(dllPath)) {
                                    userScripts.push_back(script);
                                } else if (compareTimestamps(script, dataDir + std::string("\\Engine.dll"))) {
                                    userScripts.push_back(script);
                                } else if (compareTimestamps(dataDir + std::string("\\") + std::filesystem::path(script).stem().replace_extension(".dll").string(), script)) {
                                    userScripts.push_back(script);
                                }
                                scriptsToLoad.push_back(script);
                            }
                        }

                        scriptFramework.compileScripts(userScripts, dataDir);

                        for (auto& s : scriptsToLoad)
                        {
                            std::filesystem::path p(s);
                            s = dataDir;
                            s += "/";
                            s += p.stem().replace_extension(".dll").string();
                        }
                        scriptInstances = scriptFramework.loadScripts(scriptsToLoad);
                        for (auto& script : scriptInstances) {
                            script.deserializeData(scriptFramework.getDomain(), "userScripts");
                            script.runMethod("Start");
                        }
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

            static bool show = true;
            ShowScriptManagerLayout(&show, scriptFramework, current);
        }
        frame.Render();
    }
    frame.EndFrame();

    return 0;
}
