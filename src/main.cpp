#include "Application.h"
#include "ScriptApi.h"
#include "ScriptFramework.h"
#include "ScriptHelper.h"
#include "ScriptInstance.h"

#include <filesystem>
#include <fstream>
#include <iostream>
#include <stdlib.h>
#include <string.h>

#pragma comment(lib, "Winmm.lib")
#pragma comment(lib, "Bcrypt.lib")
#pragma comment(lib, "version.lib")
#pragma comment(lib, "ws2_32")

void printCommandLineInfo()
{
    const char* info = R"(
    Command line argument is missing.
    Usage:
        -t "source directory" "output directory" - Compile framework 
        -u "source directory" "output directory" - Compile user scripts in to dlls
        -l script.dll - User scrip dll
    )";
    printf("%s", info);
}

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
        scriptFramework.load("dlls");
        scriptFramework.compileScripts(scriptFramework.createDirVector(inputDir), outputDir);
    }
    if (std::strcmp(argv[1], "-l") == 0) {
        if (arg < 3) {
            printCommandLineInfo();
            return 1;
        }
        const char* inputFile = argv[2];
        printf("Running user scripts.\n");
        printf("Input Dir: %s\n", inputFile);
        Application app;
        scriptFramework.load("dlls");
        auto script = scriptFramework.loadScript(inputFile);
        script.deserializeData(scriptFramework.getDomain(), "userScripts");
        script.init();
        script.update();
        script.printFields();
        //script.serializeData("userScripts");
    }

    return 0;
}
