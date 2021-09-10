#pragma once
#include <vector>

class ScriptCompiler
{
public:
    ScriptCompiler(const char* compilerPath);

    bool compile(const std::vector<const char*>& filenames, const char* outputName, const std::vector<const char*>& dllRefrences);

    const char* getLastError() const;
private:
    const char* compilerPath;
};