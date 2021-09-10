#include "ScriptCompiler.h"
#include <string>
#include <memory>
#include <array>
#include <sstream>

std::string error;

void execute_command(const char * cmd)
{
    std::array<char, 1024> buffer;
    const std::unique_ptr<FILE, decltype(&_pclose)> pipe(_popen(cmd, "r"), _pclose);
    if (!pipe)
    {
        return;
    }

    while (fgets(buffer.data(), static_cast<int>(buffer.size()), pipe.get()) != nullptr)
    {
        error += buffer.data();
    }

    return;
}

ScriptCompiler::ScriptCompiler(const char* compilerPath)
    : compilerPath(compilerPath)
{
}

bool ScriptCompiler::compile(const std::vector<const char*>& filenames, const char* outputName, const std::vector<const char*>& dllRefrences)
{
    std::stringstream stream;
    stream << compilerPath;
    stream << " -target:library -nologo ";
    stream << " -out:" << outputName << " ";
    if (!dllRefrences.empty())
    {
        stream << " -reference:";
        for (auto& ref : dllRefrences)
            stream << ref << " ";
    }
    for (auto& name : filenames)
        stream << name << " ";
    execute_command(stream.str().c_str());
    return false;
}

const char* ScriptCompiler::getLastError() const
{
    return error.c_str();
}
