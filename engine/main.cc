#include <iostream>

#include "mono.h"
#include "Path.h"

class ScriptAttribute
{
public:

};

class ScriptField
{
public:
    enum class Type
    { Bool, UShort, Short, UInt, Int, Double, UDouble, Undef };
    Type type;
    std::string name;
    MonoClassField* field;
    std::vector<ScriptAttribute> attributes;

    void print(std::ostream& ss)
    {
        ss << name;
        ss << (int)type;
    }
};

class ScriptClasDef
{
public:
    std::string name;
    MonoClass* monoClass;
    std::vector<ScriptField> fields;
    std::vector<ScriptAttribute> attributes;    

    const char* getParentClassName()
    {
        return "";
    }

    void print(std::ostream& ss)
    {
        ss << name;
        ss << getParentClassName();
    }
};

std::vector<ScriptClasDef> scriptClasses;

int main() {
    ScriptAssembly* coreAssembly;
    ScriptAssembly* appAssembly;
    if (!initialize("../../data/"))
    {
        printf("Error mono can not initialized");
    }

    coreAssembly = new ScriptAssembly();
    appAssembly = new ScriptAssembly();

    loadAssembly(coreAssembly, Path::fromWorkingDir("..\\game\\Debug\\EngineApi.dll"));
    loadAssembly(appAssembly, Path::fromWorkingDir("..\\game\\Debug\\Game.dll"));
    loadClasses(coreAssembly, appAssembly);

    return 0;
}
