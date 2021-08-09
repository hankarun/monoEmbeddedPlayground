#include "ScriptFramework.h"
#include "ScriptApi.h"
#include "ScriptHelper.h"

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
    const std::string dir_mono_lib = monoDir + "\\" + std::string("mono\\lib");
    const std::string dir_mono_etc = monoDir + "\\" + std::string("mono\\etc");

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
    const std::string framework_path = path + std::string("\\Engine.dll");
    const char* fp = framework_path.c_str();

    MonoAssembly* api_assembly = mono_domain_assembly_open(domain, fp);
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

std::vector<std::string> ScriptFramework::createDirVector(const std::string& inputDir)
{
    std::vector<std::string> scriptsVector;
    for (auto& p : std::filesystem::directory_iterator(inputDir)) {
        scriptsVector.push_back(p.path().u8string());
    }

    return scriptsVector;
}

void ScriptFramework::createFramework(const std::string& inputDir, const std::string& outputDir)
{
    std::vector<std::string> api_cs_path;

    for (auto& p : std::filesystem::directory_iterator(inputDir)) {
        if (p.path() == inputDir + "\\Engine.cs")
            api_cs_path.insert(api_cs_path.begin(), p.path().u8string());
        else
            api_cs_path.insert(api_cs_path.end(), p.path().u8string());
    }

    compile_script(api_cs_path, outputDir);

    //load(outputDir);
}


bool ScriptFramework::compileScripts(const std::vector<std::string>& files, const std::string outputDir)
{
    std::vector<std::string> userScript;

    for (auto& script : files) {
        if (script.find(".cs") != std::string::npos) {
            userScript.push_back(script);
            try
            {
                bool b=compile_script(userScript, outputDir, outputDir + "\\Engine.dll");
                
               
            }
            catch (...)
            {
                std::cout << "Error occured.";
            }

            userScript.clear();
        }
    }
    return true;
}


std::vector<ScriptInstance> ScriptFramework::loadScripts(const std::vector<std::string>& files)
{
    std::vector<ScriptInstance> scripts;
    for (auto& script : files) {
        if (script.find(".dll") != std::string::npos){
            if (std::filesystem::path(script).stem() != std::string("Engine"))
                scripts.push_back(ScriptInstance::load(domain, script));
        }
    }
    return scripts;
}

ScriptInstance ScriptFramework::loadScript(const std::string& filename)
{   
    return ScriptInstance::load(domain, filename);
}
