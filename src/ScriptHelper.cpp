#include "ScriptHelper.h"

std::vector<MonoClass*> GetAssemblyClassList(MonoImage * image)
{
    std::vector<MonoClass*> class_list;

    const MonoTableInfo* table_info = mono_image_get_table_info(image, MONO_TABLE_TYPEDEF);

    int rows = mono_table_info_get_rows(table_info);

    /* For each row, get some of its values */
    for (int i = 0; i < rows; i++)
    {
        MonoClass* _class = nullptr;
        uint32_t cols[MONO_TYPEDEF_SIZE];
        mono_metadata_decode_row(table_info, i, cols, MONO_TYPEDEF_SIZE);
        const char* name = mono_metadata_string_heap(image, cols[MONO_TYPEDEF_NAME]);
        printf("Class Name %s\n", name);
        const char* name_space = mono_metadata_string_heap(image, cols[MONO_TYPEDEF_NAMESPACE]);
        _class = mono_class_from_name(image, name_space, name);
        class_list.push_back(_class);
    }
    return class_list;
}

std::string replaceExtension(const std::string & file, const char * extension)
{
    std::filesystem::path p(file);
    return p.replace_extension(extension).string();
}

std::string execute_command(const char * cmd)
{
    std::array<char, 1024> buffer;
    std::string result;
    const std::unique_ptr<FILE, decltype(&_pclose)> pipe(_popen(cmd, "r"), _pclose);
    if (!pipe)
    {
        return result;
    }

    while (fgets(buffer.data(), static_cast<int>(buffer.size()), pipe.get()) != nullptr)
    {
        result += buffer.data();
    }

    return result;
}

bool compile_script(const std::vector<std::string>& scripts, const std::string & outputdir, const std::string & dll_reference)
{
    // Get paths
    const std::string dir_scripts = "data\\";
    const std::string dir_compiler = dir_scripts + "mono\\roslyn\\csc.exe";

    // Compile script
    std::string command = dir_compiler + " -target:library -nologo";
    if (!dll_reference.empty())
    {
        command += " -reference:" + dll_reference;
    }
    std::filesystem::path p(scripts.at(0));
    std::string outName = outputdir + "/" + p.filename().replace_extension(".dll").string();
    command += " -out:" + outName;
    for (auto script : scripts)
        command += " " + script;
    const std::string result = execute_command(command.c_str());

    // Log compilation output
    std::istringstream f(result);
    std::string line;
    bool compilation_result = true;
    while (std::getline(f, line))
    {
        printf("%s \n", line.c_str());
        compilation_result = false;
    }

    if (compilation_result)
    {
        return true;
    }

    return false;
}

bool compareTimestamps(const std::string & left, const std::string & right)
{
    namespace fs = std::filesystem;

    fs::path l(left);
    fs::path r(right);
    auto ltime = fs::last_write_time(l);
    auto rtime = fs::last_write_time(r);

    return rtime > ltime;
}

MonoAssembly * compile_and_load_assembly(MonoDomain * domain, const std::vector<std::string>& scripts, const std::string & outputDir, bool is_script)
{
    // Open assembly
    std::filesystem::path p(scripts.at(0));
    const std::string outName = outputDir + "/" + p.filename().replace_extension(".dll").string();

    bool result = compareTimestamps(scripts.at(0), outName);
    //bool result = false;

    if (!result)
    {
        // Ensure that the directory of the script contains the callback dll (otherwise mono will crash)
        if (is_script)
        {
            const std::string callbacks = outputDir + "/Engine.dll";

            // Compile script
            if (!compile_script(scripts, outputDir, callbacks))
            {
                printf("Failed to compile script");
                return nullptr;
            }
        }
        else
        {
            // Compile script
            if (!compile_script(scripts, outputDir))
            {
                printf("Failed to compile script");
                return nullptr;
            }
        }
    }

    return mono_domain_assembly_open(domain, outName.c_str());
}

MonoMethod * get_method(MonoImage * image, const std::string & method)
{
    // Get method description
    MonoMethodDesc* mono_method_desc = mono_method_desc_new(method.c_str(), NULL);
    if (!mono_method_desc)
    {
        printf("Failed to get method description %s", method.c_str());
        return nullptr;
    }

    // Search the method in the image
    MonoMethod* mono_method = mono_method_desc_search_in_image(mono_method_desc, image);
    if (!mono_method)
    {
        printf("Failed to get method %s", method.c_str());
        return nullptr;
    }

    return mono_method;
}
