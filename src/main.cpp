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
        scriptFramework.compileScripts(scriptFramework.createDirVector(inputDir));
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
        auto scripts = scriptFramework.loadScripts(scriptFramework.createDirVector(inputDir));
        for (auto& script : scripts)
        {
            // Load data
            script.deserializeData("temp");
            script.init();
            script.update();
            script.printFields();
            // Save data
            //script.serializeData("temp"); 

        }
    }

    return 0;
}
