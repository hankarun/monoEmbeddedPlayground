#include "mono.h"

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

#include <string>
#include <vector>

MonoDomain* domain;

static void Debug_LogFloat(float delta_time) { printf("%f\n", delta_time); }

Config config;

static void Set_Model_Name(MonoString* str) {
	config.modelName = mono_string_to_utf8(str);
}

static void SetSampleCount(int i) {
	config.sampleCount = i;
}

static void RegisterCallbacks()
{
	mono_add_internal_call("Simengine.Debug::Log", Debug_LogFloat);
	mono_add_internal_call("Simengine.Config::SetModelName", Set_Model_Name);
	mono_add_internal_call("Simengine.Config::SetSampleCount", SetSampleCount);
}

std::vector<MonoClass*> GetAssemblyClassList(MonoImage* image)
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
		const char* name_space = mono_metadata_string_heap(image, cols[MONO_TYPEDEF_NAMESPACE]);
		_class = mono_class_from_name(image, name_space, name);
		class_list.push_back(_class);
	}
	return class_list;
}

MonoMethod* get_method(MonoImage* image, const std::string& method)
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

Config* getConfig()
{
	return &config;
}

bool initialize(const Path& monoDirPath)
{
	const Path dir_mono_lib = monoDirPath.append("mono/lib");
	const Path dir_mono_etc = monoDirPath.append("mono/etc");

	// Point mono to the libs and configuration files
	mono_set_dirs(dir_mono_lib.toString().c_str(), dir_mono_etc.toString().c_str());
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

	RegisterCallbacks();
}

bool loadAssambly(const Path& filenamePath)
{
	MonoAssembly* assembly = mono_domain_assembly_open(domain, filenamePath.toString().c_str());
	if (assembly == nullptr)
	{
		return false;
	}

	MonoImage* image = mono_assembly_get_image(assembly);
	if (!image) {
		printf("Failed to get image");
		return false;
	}

	auto classes = GetAssemblyClassList(image);

	for (auto& monoClass : classes)
	{
		std::string name = mono_class_get_name(monoClass);
		printf("Class Name %s\n", name.c_str());

		if (name == "<Module>")
			continue;

		if (name == "MenuItem")
			continue;

		auto object = mono_object_new(domain, monoClass);
		if (!object) {
			mono_image_close(image);
			printf("Failed to create class instance");
			return false;
		}

		auto attributes = mono_custom_attrs_from_class(monoClass);
		if (attributes)
		{	
			auto attrClass = mono_method_get_class(attributes->attrs[0].ctor);
			auto attrObj = mono_custom_attrs_get_attr(attributes, attrClass);
			auto nameProp = mono_class_get_field_from_name(attrClass, "Name");
			MonoString* strval;
			mono_field_get_value(attrObj, nameProp, &strval);

			printf("%s", mono_string_to_utf8(strval));
		}
		


		mono_runtime_object_init(object);

		void* iter = nullptr;
		MonoMethod* method;
		while ((method = mono_class_get_methods(monoClass, &iter))) {
			printf("Method name %s\n", mono_method_get_name(method));
			method = NULL;
		}

		auto method_start = mono_class_get_method_from_name(monoClass, "OnUpdate", 0);
		if (method_start)
			mono_runtime_invoke(method_start, object, NULL, NULL);
	}
	return true;
}
