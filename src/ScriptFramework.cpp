#include "ScriptFramework.h"
#include "ScriptApi.h"

#include <vector>
#include <string>
#include <filesystem>

#include <mono/jit/jit.h>
#include <mono/metadata/appdomain.h>
#include <mono/metadata/assembly.h>
#include <mono/metadata/debug-helpers.h>
#include <mono/metadata/environment.h>
#include <mono/metadata/exception.h>
#include <mono/metadata/loader.h>
#include <mono/metadata/mono-config.h>
#include <mono/metadata/mono-debug.h>
#include <mono/metadata/mono-gc.h>
#include <mono/metadata/threads.h>

bool ScriptFramework::initialize(const std::string& monoDir)
{
    const std::string dir_scripts = "data\\";
    const std::string dir_mono_lib = dir_scripts + std::string("mono\\lib");
    const std::string dir_mono_etc = dir_scripts + std::string("mono\\etc");

    // Point mono to the libs and configuration files
    mono_set_dirs(dir_mono_lib.c_str(), dir_mono_etc.c_str());
    // Initialise a domain
    domain = mono_jit_init_version("Simengine", "v4.0.30319");
    if (!domain) {
        printf("mono_jit_init failed");
        return false;
    }

    if (!mono_domain_set(domain, false)) {
        printf("mono_domain_set failed");
        return false;
    }

    // soft debugger needs this
    mono_thread_set_main(mono_thread_current());

    return true;
}

bool ScriptFramework::load(const std::string& path)
{
    MonoAssembly* api_assembly = mono_domain_assembly_open(domain, "temp/Engine.dll");
    if (!api_assembly) {
        printf("Failed to get api assembly");
        return false;
    }

    // Get image from script assembly
    MonoImage* callbacks_image = mono_assembly_get_image(api_assembly);
    if (!callbacks_image) {
        printf("Failed to get callbacks image");
        return false;
    }

    // Register static callbacks
    RegisterCallbacks();

    return true;
}

void ScriptFramework::createFramework(const std::string& inputDir, const std::string& outputDir)
{
    // Alınan dizin içerisindeki tüm cs dosyalarının bulunup framework dll'inin oluşturulması
    std::vector<std::string> api_cs_path;

    for (auto& p : std::filesystem::directory_iterator(inputDir)) {
        // ScriptHelper'da framework vektörün ilk elemanının ismi ile oluşturulduğundan dolayı
        if (p.path() == "scripts\\Engine.cs")
            api_cs_path.insert(api_cs_path.begin(), p.path().u8string());
        else
            api_cs_path.insert(api_cs_path.end(), p.path().u8string());
    }

    printf("compiling assembly...\n");
    load(outputDir);
}

bool ScriptFramework::compileScripts(const std::vector<std::string>& files)
{
    return false;
}

std::vector<ScriptInstance> ScriptFramework::loadScripts(const std::vector<std::string>& files)
{
    return std::vector<ScriptInstance>();
}
