#pragma once
#include <vector>

class ScriptCompiler
{
public:
    ScriptCompiler(const char* compilerPath);

    bool compile(const char* filename, const char* outputdir, const std::vector<const char*>& dllRefrences);

    const char* getLastError() const;
private:
    const char* compilerPath;
};