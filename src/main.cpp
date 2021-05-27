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

int main(int arg, char* argv[])
{
    ScriptFramework scriptFramework;
    scriptFramework.initialize("data");

    const char* inputFile = "temp/SampleUserScript.dll";
    printf("Running user scripts.\n");
    printf("Input Dir: %s\n", inputFile);
    Application app;
    scriptFramework.load("dlls");

    std::vector<ScriptInstance> scriptInstances;
    {
        auto script = scriptFramework.loadScript(inputFile);
        script.deserializeData(scriptFramework.getDomain(), "userScripts");
        script.init();
        scriptInstances.push_back(script);
    }

    ImGuiFrame frame;
    frame.Initialize();

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
        {
            if (ImGui::BeginMainMenuBar()) {
                if (ImGui::BeginMenu("File")) {
                    if (ImGui::MenuItem("Update Engine Framework")) {
                        // Compile engine framework
                        //const char* inputDir = argv[2];
                        //const char* outputDir = argv[3];
                        //printf("Compiling framework.\n");
                        //printf("Input Dir: %s\n", inputDir);
                        //printf("Output Dir: %s\n", outputDir);
                        //scriptFramework.createFramework(inputDir, outputDir);
                    }
                    if (ImGui::MenuItem("Load User Scripts")) {
                        // Load Scripts
                        // Compile if engine is newer than dll
                        // Compile if cs is newer than dll
                        // Compile if dll not found

                        //const char* inputDir = argv[2];
                        //const char* outputDir = argv[3];
                        //printf("Compiling user scripts.\n");
                        //printf("Input Dir: %s\n", inputDir);
                        //printf("Output Dir: %s\n", outputDir);
                        //scriptFramework.load("dlls");
                        //scriptFramework.compileScripts(scriptFramework.createDirVector(inputDir), outputDir);
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

            if (ImGui::Begin("Script Manager")) {
                // For every script loaded
                for (int i = 0; i < scriptInstances.size(); ++i) {
                    if (ImGui::Selectable(scriptInstances.at(i).getClassName(), i == selected))
                        selected = i;
                }
            }
            ImGui::End();

            if (ImGui::Begin("Output Window")) {
                ImGui::Text("Scrpit outputs");
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

        frame.Render();
    }
    frame.EndFrame();

    return 0;
}
