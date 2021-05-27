#include "ScriptInstance.h"
#include "ImGuiFrame.h"
#include "ScriptHelper.h"
#include <filesystem>
#include <fstream>

#include <../rapidjson/document.h>
#include <../rapidjson/prettywriter.h>
#include <../rapidjson/stringbuffer.h>
#include <../rapidjson/writer.h>

ScriptInstance ScriptInstance::load(MonoDomain* domain, const std::string& file_path)
{
    ScriptInstance script;
    std::filesystem::path p(file_path);
    const std::string class_name = p.stem().string();

    script.assembly = mono_domain_assembly_open(domain, file_path.c_str());
    if (!script.assembly) {
        printf("Failed to load assembly");
        return script;
    }

    // Get image from script assembly
    script.image = mono_assembly_get_image(script.assembly);
    if (!script.image) {
        printf("Failed to get image");
        return script;
    }

    //auto classes = GetAssemblyClassList(script.image);
    //printf("Class count %d\n", classes.size());

    // Get the class
    script.klass = mono_class_from_name(script.image, "", class_name.c_str());
    if (!script.klass) {
        mono_image_close(script.image);
        printf("Failed to get class");
        return script;
    }

    // Create class instance
    script.object = mono_object_new(domain, script.klass);
    if (!script.object) {
        mono_image_close(script.image);
        printf("Failed to create class instance");
        return script;
    }

    // Get methods
    if (!(mono_class_num_methods(script.klass))) {
        script.method_start = get_method(script.image, class_name + ":Start()");
        script.method_update = get_method(script.image, class_name + ":Update()");
    }

    // Set entity handle
    //if (!script.SetValue(script_component->GetEntity(), "_internal_entity_handle"))
    //{
    //    mono_image_close(script.image);
    //    LOG_ERROR("Failed to set entity handle");
    //    return SCRIPT_NOT_LOADED;
    //}

    //// Set transform handle
    //if (!script.SetValue(script_component->GetTransform(), "_internal_transform_handle"))
    //{
    //    mono_image_close(script.image);
    //    LOG_ERROR("Failed to set transform handle");
    //    return SCRIPT_NOT_LOADED;
    //}

    // Call the default constructor
    mono_runtime_object_init(script.object);
    if (!script.object) {
        mono_image_close(script.image);
        printf("Failed to run class constructor");
        return script;
    }

    //script.output_methods();
    //script.output_properties();
    script.printFields();

    // Return script id
    return script;
}

void ScriptInstance::serializeData(const std::string& json_path) const
{
    const char* name = mono_class_get_name(klass);
    using namespace rapidjson;

    StringBuffer sb;
    PrettyWriter<StringBuffer> writer(sb);

    writer.StartObject();
    writer.Key("script");
    writer.String(name);

    MonoClassField* field;
    void* iter = NULL;
    while ((field = mono_class_get_fields(klass, &iter))) {
        if (mono_type_get_type(mono_field_get_type(field)) == MONO_TYPE_I4) {
            int value = 0;
            mono_field_get_value(object, field, &value);
            writer.Key(mono_field_get_name(field));
            writer.Int(value);
        }

        else if (mono_type_get_type(mono_field_get_type(field)) == MONO_TYPE_STRING) {
            MonoString* strval;
            mono_field_get_value(object, field, &strval);
            char* p = mono_string_to_utf8(strval);
            int size = mono_string_length(strval);
            writer.Key(mono_field_get_name(field));
            writer.String(p);
            mono_free(p);
        }

        else if (mono_type_get_type(mono_field_get_type(field)) == MONO_TYPE_R8) {
            double value = 0;
            mono_field_get_value(object, field, &value);
            writer.Key(mono_field_get_name(field));
            writer.Double(value);
        }

        else if (mono_type_get_type(mono_field_get_type(field)) == MONO_TYPE_BOOLEAN) {
            bool b = true;
            mono_field_get_value(object, field, &b);
            writer.Key(mono_field_get_name(field));
            writer.Bool(b);
        }
    }
    writer.EndObject();

    std::string json(sb.GetString());
    std::string className = name;
    std::ofstream os(json_path + "\\" + className + ".json");
    os << json;
    os.close();
}

void ScriptInstance::deserializeData(MonoDomain* domain, const std::string& json_path)
{
    const std::string name = mono_class_get_name(klass);
    std::ifstream file(json_path + "\\" + name + ".json");
    std::ostringstream oss;
    oss << file.rdbuf();
    const std::string data = oss.str();

    rapidjson::Document doc;
    doc.Parse(data.c_str());

    if (doc.Parse(data.c_str()).HasParseError())
        printf("Failed to parse serialized data");

    for (rapidjson::Value::ConstMemberIterator itr = doc.MemberBegin(); itr != doc.MemberEnd(); ++itr) {
        if (itr->value.IsString()) {
            std::string variableName = itr->name.GetString();
            if (variableName != "script") {
                std::string strValue = itr->value.GetString();
                SetStringValue(domain, &strValue, variableName);
            }
        } else if (itr->value.IsInt()) {
            int intValue = itr->value.GetInt();
            int* intp = &intValue;
            SetValue(intp, itr->name.GetString());
        } else if (itr->value.IsDouble()) {
            double doubleValue = itr->value.GetDouble();
            double* dp = &doubleValue;
            SetValue(dp, itr->name.GetString());
        } else if (itr->value.IsBool()) {
            bool boolValue = itr->value.GetBool();
            bool* bp = &boolValue;
            SetValue(bp, itr->name.GetString());
        }
    }
}

void ScriptInstance::updateVariablesOnGUI(MonoDomain* domain, const std::string& json_path)
{
    const std::string name = mono_class_get_name(klass);
    std::ifstream file(json_path + "\\" + name + ".json");
    std::ostringstream oss;
    oss << file.rdbuf();
    const std::string data = oss.str();

    rapidjson::Document doc;
    doc.Parse(data.c_str());

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

        {
            ImGui::Begin("MONO");

            MonoClassField* field;
            void* iter = NULL;
            while ((field = mono_class_get_fields(klass, &iter))) {
                if (mono_type_get_type(mono_field_get_type(field)) == MONO_TYPE_I4) {
                    int value = 0;
                    mono_field_get_value(object, field, &value);
                    if (ImGui::DragInt(mono_field_get_name(field), &value))
                        mono_field_set_value(object, field, &value);
                }

                else if (mono_type_get_type(mono_field_get_type(field)) == MONO_TYPE_STRING) {
                    MonoString* strval;
                    mono_field_get_value(object, field, &strval);
                    char* p = mono_string_to_utf8(strval);
                    int size = mono_string_length(strval);
                    if (ImGui::InputText(mono_field_get_name(field), p, size)) {
                        std::string data(p);
                        SetStringValue(domain, &data, mono_field_get_name(field));
                    }
                    mono_free(p);
                }

                else if (mono_type_get_type(mono_field_get_type(field)) == MONO_TYPE_R8) {
                    double value = 0;
                    mono_field_get_value(object, field, &value);
                    if (ImGui::DragScalarN(mono_field_get_name(field), ImGuiDataType_Double, (void*)&value, 1, 1))
                        mono_field_set_value(object, field, &value);
                }

                else if (mono_type_get_type(mono_field_get_type(field)) == MONO_TYPE_BOOLEAN) {
                    bool b = true;
                    mono_field_get_value(object, field, &b);
                    if (ImGui::Checkbox(mono_field_get_name(field), &b))
                        mono_field_set_value(object, field, &b);
                }
            }
            
            ImGui::End();
        }

        {
            ImGui::Begin("Help");
            ImGui::Text("User Guide: ");

            ImGui::BulletText("Update script variables using input bars.");
            ImGui::BulletText("Press Update button to save.");
            ImGui::BulletText("You can use +- on numerical values.");
            ImGui::BulletText("Use ESCAPE to undo changes.");
            ImGui::BulletText("You can apply arithmetic operators \n +, *, / on numerical values.");

            ImGui::End();
        }

        frame.Render();
    }
    frame.EndFrame();
}

inline void ScriptInstance::printFields()
{
    MonoClassField* field;
    void* iter = NULL;

    while ((field = mono_class_get_fields(klass, &iter))) {
        auto fieldFlags = mono_field_get_flags(field) & MONO_FIELD_ATTR_FIELD_ACCESS_MASK;

        switch (fieldFlags) {
        case MONO_FIELD_ATTR_PRIVATE:
            printf("private - ");
            break;
        case MONO_FIELD_ATTR_FAM_AND_ASSEM:
            printf("protected internal - ");
            break;
        case MONO_FIELD_ATTR_ASSEMBLY:
            printf("internal - ");
            break;
        case MONO_FIELD_ATTR_FAMILY:
            printf("protected - ");
            break;
        case MONO_FIELD_ATTR_PUBLIC:
            printf("public - ");
            break;
        default:
            printf("private - ");
        }

        printf("Field: %s, flags 0x%x", mono_field_get_name(field), mono_field_get_flags(field));

        if (mono_type_get_type(mono_field_get_type(field)) == MONO_TYPE_I4) {
            int value = 0;
            mono_field_get_value(object, field, &value);
            printf(" Value (int32) : %d\n", value);
        }

        else if (mono_type_get_type(mono_field_get_type(field)) == MONO_TYPE_STRING) {
            MonoString* strval;
            mono_field_get_value(object, field, &strval);
            char* p = mono_string_to_utf8(strval);
            printf(" Value of str is: %s\n", p);
            mono_free(p);
        }

        else if (mono_type_get_type(mono_field_get_type(field)) == MONO_TYPE_R8) {
            double value = 0;
            mono_field_get_value(object, field, &value);
            printf(" Value (Double): %f\n", value);
        }

        else if (mono_type_get_type(mono_field_get_type(field)) == MONO_TYPE_BOOLEAN) {
            bool b = true;
            mono_field_get_value(object, field, &b);
            printf(" Value (Boolean): %d\n", b);
        }
    }
}

void ScriptInstance::output_methods()
{
    MonoMethod* method;
    void* iter = NULL;

    while ((method = mono_class_get_methods(klass, &iter))) {
        uint32_t flags, iflags;
        flags = mono_method_get_flags(method, &iflags);
        printf("Method: %s, flags 0x%x, iflags 0x%x\n",
            mono_method_get_name(method), flags, iflags);
    }
}

void ScriptInstance::output_properties()
{
    MonoProperty* prop;
    void* iter = NULL;

    while ((prop = mono_class_get_properties(klass, &iter))) {
        printf("Property: %s, flags 0x%x\n", mono_property_get_name(prop),
            mono_property_get_flags(prop));
    }
}

void ScriptInstance::init()
{
    if (method_start)
        mono_runtime_invoke(method_start, NULL, NULL, NULL);
}

void ScriptInstance::update()
{
    if (method_update)
        mono_runtime_invoke(method_update, NULL, NULL, NULL);
}
