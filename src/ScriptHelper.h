#pragma once
#include <array>
#include <iostream>
#include <filesystem>
#include <sstream>

#include <mono/jit/jit.h>
#include <mono/metadata/assembly.h>
#include <mono/metadata/loader.h>
#include <mono/metadata/threads.h>
#include <mono/metadata/mono-gc.h>
#include <mono/metadata/mono-config.h>
#include <mono/metadata/mono-debug.h>
#include <mono/metadata/environment.h>
#include <mono/metadata/debug-helpers.h>
#include <mono/metadata/exception.h>
#include <mono/metadata/appdomain.h>

std::vector<MonoClass*> GetAssemblyClassList(MonoImage * image);

std::string replaceExtension(const std::string& file, const char* extension);

std::string execute_command(const char* cmd);

bool compile_script(const std::vector<std::string>& scripts, const std::string& outputdir, const std::string& dll_reference = "");

bool compareTimestamps(const std::string& left, const std::string& right);

MonoAssembly* compile_and_load_assembly(MonoDomain* domain, const std::vector<std::string>& scripts, const std::string& outputDir, bool is_script = true);

MonoMethod* get_method(MonoImage* image, const std::string& method);

