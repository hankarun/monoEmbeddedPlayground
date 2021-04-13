#include <iostream>
#include <fstream>
#include <filesystem>
#include "ScriptHelper.h"
#include "Application.h"
#include "ScriptApi.h"
#include <mono/metadata/attrdefs.h>
#include <mono/metadata/object.h>
#include <mono/metadata/environment.h>
#include <mono/metadata/assembly.h>
#include <mono/metadata/debug-helpers.h>
#include <mono/metadata/mono-config.h>
#include <string.h>
#include <stdlib.h>
#include <../rapidjson/document.h>
#include <../rapidjson/writer.h>
#include <../rapidjson/stringbuffer.h>
#include <../rapidjson/prettywriter.h>

#pragma comment(lib, "Winmm.lib")
#pragma comment(lib, "Bcrypt.lib")
#pragma comment(lib, "version.lib")
#pragma comment(lib, "ws2_32")

MonoDomain* m_domain;
Application app;

struct ScriptInstance
{
    MonoAssembly* assembly = nullptr;
    MonoImage* image = nullptr;
    MonoClass* klass = nullptr;
    MonoObject* object = nullptr;
    MonoMethod* method_start = nullptr;
    MonoMethod* method_update = nullptr;

    template<class T>
    bool SetValue(T* value, const std::string& name)
    {
        if (MonoClassField* field = mono_class_get_field_from_name(klass, name.c_str()))
        {
            mono_field_set_value(object, field, value);
            return true;
        }

        printf("Failed to set value for field %s", name.c_str());
        return false;
    }
};

bool Initialize()
{
    const std::string dir_scripts = "data\\";
    const std::string dir_mono_lib = dir_scripts + std::string("mono\\lib");
    const std::string dir_mono_etc = dir_scripts + std::string("mono\\etc");

    // Point mono to the libs and configuration files
    mono_set_dirs(dir_mono_lib.c_str(), dir_mono_etc.c_str());
    // Initialise a domain
    m_domain = mono_jit_init_version("Simengine", "v4.0.30319");
    if (!m_domain)
    {
        printf("mono_jit_init failed");
        return false;
    }

    if (!mono_domain_set(m_domain, false))
    {
        printf("mono_domain_set failed");
        return false;
    }

    // soft debugger needs this
    mono_thread_set_main(mono_thread_current());

    return true;
}

bool OpenCompiledAssembly()
{
    MonoAssembly* api_assembly = mono_domain_assembly_open(m_domain, "temp/Engine.dll");
    if (!api_assembly)
    {
        printf("Failed to get api assembly");
        return false;
    }

    // Get image from script assembly
    MonoImage* callbacks_image = mono_assembly_get_image(api_assembly);
    if (!callbacks_image)
    {
        printf("Failed to get callbacks image");
        return false;
    }

    // Register static callbacks
    RegisterCallbacks();

    return true;
}

bool CompileApiAssembly(std::vector<std::string> api_cs)
{
    //// Get callbacks assembly
    //std::vector<std::string> api_cs = {
    //    "scripts/Engine.cs",
    //    "scripts/Time.cs",
    //    "scripts/Debug.cs"
    //};

    MonoAssembly* api_assembly = compile_and_load_assembly(m_domain, api_cs, false);
    if (!api_assembly)
    {
        printf("Failed to get api assembly");
        return false;
    }

    // Get image from script assembly
    MonoImage* callbacks_image = mono_assembly_get_image(api_assembly);
    if (!callbacks_image)
    {
        printf("Failed to get callbacks image");
        return false;
    }

    // Register static callbacks
    RegisterCallbacks();

    return true;
}

void output_fields(MonoClass* klass, MonoObject* obj) {
    MonoClassField* field;
    void* iter = NULL;

    while ((field = mono_class_get_fields(klass, &iter))) {
        auto fieldFlags = mono_field_get_flags(field) & MONO_FIELD_ATTR_FIELD_ACCESS_MASK;

        switch (fieldFlags) {
        case MONO_FIELD_ATTR_PRIVATE:
            printf("private - "); break;
        case MONO_FIELD_ATTR_FAM_AND_ASSEM:
            printf("protected internal - "); break;
        case MONO_FIELD_ATTR_ASSEMBLY:
            printf("internal - "); break;
        case MONO_FIELD_ATTR_FAMILY:
            printf("protected - "); break;
        case MONO_FIELD_ATTR_PUBLIC:
            printf("public - "); break;
        default:
            printf("private - ");
        }

        printf("Field: %s, flags 0x%x", mono_field_get_name(field), mono_field_get_flags(field));

        if (mono_type_get_type(mono_field_get_type(field)) == MONO_TYPE_I4)
        {
            int value = 0;
            mono_field_get_value(obj, field, &value);
            printf(" Value (int32) : %d\n", value);
        }

        if (mono_type_get_type(mono_field_get_type(field)) == MONO_TYPE_STRING)
        {
            MonoString* strval;
            mono_field_get_value(obj, field, &strval);
            char* p = mono_string_to_utf8(strval);
            printf(" Value of str is: %s\n", p);
            mono_free(p);
        }
    }
}

void output_methods(MonoClass* klass) {
    MonoMethod* method;
    void* iter = NULL;

    while ((method = mono_class_get_methods(klass, &iter))) {
        uint32_t flags, iflags;
        flags = mono_method_get_flags(method, &iflags);
        printf("Method: %s, flags 0x%x, iflags 0x%x\n",
            mono_method_get_name(method), flags, iflags);
    }
}

void output_properties(MonoClass* klass) {
    MonoProperty* prop;
    void* iter = NULL;

    while ((prop = mono_class_get_properties(klass, &iter))) {
        printf("Property: %s, flags 0x%x\n", mono_property_get_name(prop),
            mono_property_get_flags(prop));
    }
}


ScriptInstance Load(const std::string& file_path)
{
    ScriptInstance script;
    std::filesystem::path p(file_path);
    const std::string class_name = p.stem().string();

    script.assembly = compile_and_load_assembly(m_domain, { file_path });
    if (!script.assembly)
    {
        printf("Failed to load assembly");
        return script;
    }

    // Get image from script assembly
    script.image = mono_assembly_get_image(script.assembly);
    if (!script.image)
    {
        printf("Failed to get image");
        return script;
    }

    //auto classes = GetAssemblyClassList(script.image);
    //printf("Class count %d\n", classes.size());

    // Get the class
    script.klass = mono_class_from_name(script.image, "", class_name.c_str());
    if (!script.klass)
    {
        mono_image_close(script.image);
        printf("Failed to get class");
        return script;
    }


    // Create class instance
    script.object = mono_object_new(m_domain, script.klass);
    if (!script.object)
    {
        mono_image_close(script.image);
        printf("Failed to create class instance");
        return script;
    }


    // Get methods
    if (!(mono_class_num_methods(script.klass)))
    {
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
    if (!script.object)
    {
        mono_image_close(script.image);
        printf("Failed to run class constructor");
        return script;
    }

    //output_methods(script.klass);
    //output_properties(script.klass);
    //output_fields(script.klass, script.object);

    // Return script id
    return script;
}

template <typename T> std::string stringify(T x) {
    std::stringstream ss;
    ss << x;
    return ss.str();
}

//ScriptInstance Create(const std::string& file_path, std::string serializedComponent)
//{
//    using namespace rapidjson;
//    using namespace std;
//
//    struct Handler {
//        std::string type;
//        std::string data;
//
//        Handler() : type(), data() {}
//        bool Null() { type = "null"; data.clear(); return true; }
//        bool Bool(bool b) { type = "bool "; data = b ? "true" : "false"; return true; }
//        bool Int(int i) { type = "int "; data = stringify(i); return true; }
//        bool Uint(unsigned u) { type = "int "; data = stringify(u); return true; }
//        bool Int64(int64_t i) { type = "int64 "; data = stringify(i); return true; }
//        bool Uint64(uint64_t u) { type = "ulong "; data = stringify(u); return true; }
//        bool Double(double d) { type = "double "; data = stringify(d); return true; }
//        bool RawNumber(const char* str, SizeType length, bool) { type = "int "; data = std::string(str, length); return true; }
//        bool String(const char* str, SizeType length, bool) { type = "string "; data = std::string(str, length); return true; }
//        bool StartObject() { type = "StartObject"; data.clear(); return true; }
//        bool Key(const char* str, SizeType length, bool) { type = "Key:"; data = std::string(str, length); return true; }
//        bool EndObject(SizeType memberCount) { type = "EndObject:"; data = stringify(memberCount); return true; }
//        bool StartArray() { type = "StartArray"; data.clear(); return true; }
//        bool EndArray(SizeType elementCount) { type = "EndArray:"; data = stringify(elementCount); return true; }
//    private:
//        Handler(const Handler& noCopyConstruction);
//        Handler& operator=(const Handler& noAssignment);
//    };
//
//    Handler handler;
//    Reader reader;
//
//    char* serializedData = const_cast<char*>(serializedComponent.c_str());
//    rapidjson::StringStream ss(serializedData);
//    std::filesystem::path p(file_path);
//    const std::string class_name = p.stem().string();
//
//    fstream ScriptFile;
//    ScriptFile.open(file_path, ios::out);
//    ScriptFile << "using System;\nusing System.Runtime.CompilerServices;\nusing System.Runtime.InteropServices;\nusing Simengine;\n\n";
//    ScriptFile << "public class " + class_name + " : MonoSystem\n{\n";
//
//    string key = "";
//
//    reader.IterativeParseInit();
//    while (!reader.IterativeParseComplete()) {
//        reader.IterativeParseNext<kParseDefaultFlags>(ss, handler);
//        if (handler.type == "StartObject" || handler.type == "EndObject:")
//        {
//            continue;
//        }
//        else if (handler.type == "Key:")
//        {
//            key = handler.data.c_str();
//            continue;
//        }
//        else
//        {
//            if (key == "script") { continue; }
//
//            if (handler.type == "string ")
//            {
//                ScriptFile << "public " + handler.type + key + " = " + "\"" + handler.data + "\"" + ";" << endl;
//            }
//            else
//            {
//                ScriptFile << "public " + handler.type + key + " = " + handler.data + ";" << endl;
//            }
//        }
//    }
//
//    ScriptFile << "}" << endl;
//    ScriptFile.close();
//
//    ScriptInstance script = Load(file_path);
//    return script;
//}

std::string serializeScriptInstance(ScriptInstance script)
{
    const char* name = mono_class_get_name(script.klass);
    using namespace rapidjson;

    StringBuffer sb;
    PrettyWriter<StringBuffer> writer(sb);

    writer.StartObject();
    writer.Key("script");
    writer.String(name);

    MonoClassField* field;
    void* iter = NULL;
    while ((field = mono_class_get_fields(script.klass, &iter)))
    {
        if (mono_type_get_type(mono_field_get_type(field)) == MONO_TYPE_I4)
        {
            int value = 0;
            mono_field_get_value(script.object, field, &value);
            writer.Key(mono_field_get_name(field));
            writer.Int(value);
        }

        if (mono_type_get_type(mono_field_get_type(field)) == MONO_TYPE_STRING)
        {
            MonoString* strval;
            mono_field_get_value(script.object, field, &strval);
            char* p = mono_string_to_utf8(strval);
            int size = mono_string_length(strval);
            writer.Key(mono_field_get_name(field));
            writer.String(p);
            mono_free(p);
        }
    }
    writer.EndObject();

    return sb.GetString();
}


int main()
{
    Initialize();

    // Eğer daha önceden derlendiyse bir daha framework derlenmesin
    if (!(mono_domain_assembly_open(m_domain, "temp/Engine.dll")))
    {
        // Alınan dizin içerisindeki tüm cs dosyalarının bulunup framework dll'inin oluşturulması
        std::string scripts_path;
        printf("path to c# scripts: ");
        std::cin >> scripts_path;
        std::vector<std::string> api_cs_path;

        for (auto& p : std::filesystem::directory_iterator(scripts_path))
        {
            // ScriptHelper'da framework vektörün ilk elemanının ismi ile oluşturulduğundan dolayı
            if (p.path() == "scripts\\Engine.cs")
                api_cs_path.insert(api_cs_path.begin(), p.path().u8string());
            else
                api_cs_path.insert(api_cs_path.end(), p.path().u8string());
        }

        printf("compiling assembly...\n");
        CompileApiAssembly(api_cs_path);
    }

    OpenCompiledAssembly();

    // Engine dll'i kullanılarak kullanıcı scriptlerinin dll dosyalarının oluşturulması
    std::string user_scripts_path;
    printf("path to user c# scripts: ");
    std::cin >> user_scripts_path;
    std::vector<std::string> user_cs_path;

    for (auto& p : std::filesystem::directory_iterator(user_scripts_path))
    {
        ScriptInstance script = Load(p.path().u8string());
    }


    //std::string serializedComponent = serializeScriptInstance(Load("scripts/SampleScript.cs"));
    //printf("%s\n", serializedComponent.c_str());

    //// Burada serializedComponent tekrar objeye dönsün
    //ScriptInstance newScript = Create("scripts/NewScript.cs", serializedComponent);

    //mono_runtime_invoke(script.method_start, script.object, nullptr, nullptr);
    //mono_runtime_invoke(script.method_update, script.object, nullptr, nullptr);
}
