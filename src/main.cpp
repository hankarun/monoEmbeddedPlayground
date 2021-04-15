#include "Application.h"
#include "ScriptApi.h"
#include "ScriptHelper.h"
#include "ScriptInstance.h"
#include "ScriptFramework.h"

#include <filesystem>
#include <fstream>
#include <iostream>
#include <stdlib.h>
#include <string.h>

#pragma comment(lib, "Winmm.lib")
#pragma comment(lib, "Bcrypt.lib")
#pragma comment(lib, "version.lib")
#pragma comment(lib, "ws2_32")


//template <typename T> std::string stringify(T x) {
//    std::stringstream ss;
//    ss << x;
//    return ss.str();
//}

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


//void CompileUserScripts(const char* inputDir, const char* outputDir)
//{
//    // Engine dll'i kullanılarak kullanıcı scriptlerinin dll dosyalarını oluştur
//    std::vector<std::string> user_cs_path;
//
//    int dllSwitch;
//    printf("Type 0 for compiling all scripts in one dll file \nType 1 for seperate dll for each script\n");
//    std::cin >> dllSwitch;
//    switch (dllSwitch) {
//    case 0:
//        // Tüm kullanıcı scriptlerini tek bir dll'de birleştir
//        for (auto& p : std::filesystem::directory_iterator(inputDir)) {
//            user_cs_path.insert(user_cs_path.end(), p.path().u8string());
//        }
//        compile_script(user_cs_path, "temp/Engine.dll");
//        break;
//
//    case 1:
//        // Her bir kullanıcı scripti için ayrı bir dll oluştur
//        for (auto& p : std::filesystem::directory_iterator(inputDir)) {
//            user_cs_path.insert(user_cs_path.end(), p.path().u8string());
//            compile_script(user_cs_path, outputDir + std::string("/Engine.dll"));
//            user_cs_path.clear();
//        }
//        break;
//    }
//}

//void LoadDLLs()
//{
//    // Derlenen dll'leri yükle çalıştır
//    std::string dll_path;
//    printf("path to dlls: ");
//    std::cin >> dll_path;
//    for (auto& p : std::filesystem::directory_iterator(dll_path)) {
//        if (p.path() == "temp\\.gitignore" || p.path() == "temp\\Engine.dll")
//            continue;
//
//        std::filesystem::path path(p);
//        const std::string class_name = path.stem().string();
//
//        MonoAssembly* assembly = mono_domain_assembly_open(m_domain, p.path().u8string().c_str());
//        MonoImage* image = mono_assembly_get_image(assembly);
//        MonoClass* klass = mono_class_from_name(image, "", class_name.c_str());
//        MonoObject* object = mono_object_new(m_domain, klass);
//
//        mono_runtime_object_init(object);
//
//        output_methods(klass);
//    }
//}

void printCommandLineInfo()
{
    const char* info = R"(
    Command line argument is missing.
    Usage:
        -t "source directory" "output directory" - Compile framework 
        -u "source directory" "output directory" - Compile user scripts in to dlls
        -l "source directory" - Load managed dlls and run scripts
    )";
    printf("%s", info);
}


// Serialize and deserialize functionality need to be implemented in a way
int main(int arg, char* argv[])
{
    if (arg == 1) {
        printCommandLineInfo();
        return 1;
    }
    ScriptFramework scriptFramework;
    scriptFramework.initialize("data");
    if (std::strcmp(argv[1], "-t") == 0) {
        if (arg < 4) {
            printCommandLineInfo();
            return 1;
        }
        const char* inputDir = argv[2];
        const char* outputDir = argv[3];
        printf("Compiling framework.\n");
        printf("Input Dir: %s\n", inputDir);
        printf("Output Dir: %s\n", outputDir);
        scriptFramework.createFramework(inputDir, outputDir);
    }
    if (std::strcmp(argv[1], "-u") == 0) {
        if (arg < 4) {
            printCommandLineInfo();
            return 1;
        }
        const char* inputDir = argv[2];
        const char* outputDir = argv[3];
        printf("Compiling user scripts.\n");
        printf("Input Dir: %s\n", inputDir);
        printf("Output Dir: %s\n", outputDir);
        scriptFramework.load("temp");
        scriptFramework.compileScripts({});
        //mono_domain_assembly_open(m_domain, "temp/Engine.dll");
        //CompileUserScripts(inputDir, outputDir);
    }
    if (std::strcmp(argv[1], "-l") == 0) {
        if (arg < 3) {
            printCommandLineInfo();
            return 1;
        }
        const char* inputDir = argv[2];
        const char* outputDir = argv[3];
        printf("Running user scripts.\n");
        printf("Input Dir: %s\n", inputDir);
        Application app;
        scriptFramework.load("temp");
        auto scripts = scriptFramework.loadScripts({});
        for (auto& script : scripts)
        {
            // Load data
            script.deserializeData("");
            script.init();
            script.update();
            auto data = script.serializeData();
            // Save data
        }
        //mono_domain_assembly_open(m_domain, "temp/Engine.dll");
        //LoadDLLs();
    }

    return 0;
}
