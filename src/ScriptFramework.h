#pragma once
#include <string>
#include <vector>
#include <mono/utils/mono-forward.h>

#include "ScriptInstance.h"

class ScriptFramework
{
public:
    bool initialize(const std::string& monoDir);
    bool load(const std::string& path);

    std::vector<std::string> createDirVector(const std::string& inputDir);
    bool EndsWith(std::string str, std::string suffix);
    void createFramework(const std::string& inputDir, const std::string& outputDir);
    bool compileScripts(const std::vector<std::string>& files, const std::string outputDir);
    std::vector<std::string> compileScripts2(const std::vector<std::string>& files, const std::string outputDir);
    std::vector<ScriptInstance> loadScripts(const std::vector<std::string>& files);
    ScriptInstance loadScript(const std::string& filename);

    MonoDomain* getDomain() const { return domain; }
private:
    MonoDomain* domain;
};

class ScriptCompiler
{
public:
    bool compile(const char* filename, const char* outputdir);

    const char* getLastError() const;
private:
    std::string error;
};
