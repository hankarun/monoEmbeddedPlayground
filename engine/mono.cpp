#include "mono.h"



#include <string>
#include <vector>
#include <iostream>

MonoDomain* domain;

static void Debug_LogFloat(float delta_time) { printf("%f\n", delta_time); }

static void RegisterCallbacks()
{
	mono_add_internal_call("Simengine.Debug::Log", Debug_LogFloat);
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
	return true;
}

ScriptFieldType MonoTypeToScriptFieldType(MonoType* monoType)
{
	std::string typeName = mono_type_get_name(monoType);

	auto it = s_ScriptFieldTypeMap.find(typeName);
	if (it == s_ScriptFieldTypeMap.end())
	{
		return ScriptFieldType::None;
	}

	return it->second;
}

struct FieldDef
{
	std::string name;
	ScriptFieldType fieldType;
	uint32_t flags;
};

FieldDef loadFieldDef(MonoClassField* field)
{
	FieldDef def;
	def.name = mono_field_get_name(field);
	def.flags = mono_field_get_flags(field);
	def.fieldType = MonoTypeToScriptFieldType(mono_field_get_type(field));
	return def;
}


std::ostream& operator<<(std::ostream& s, const FieldDef& item)
{
	s << "Field Name: " << item.name << " type: " << (int)item.fieldType;
	s << " Flags: ";
	if (item.flags & FIELD_ATTRIBUTE_PUBLIC)
		s << "Public, ";
	if (item.flags & FIELD_ATTRIBUTE_PRIVATE)
		s << "Private, ";
	if (item.flags & FIELD_ATTRIBUTE_STATIC)
		s << "Static ";
	return s;
}



bool loadAssembly(ScriptAssembly* _assembly, const Path& path)
{
	_assembly->assembly = mono_domain_assembly_open(domain, path.toString().c_str());
	if (_assembly->assembly == nullptr)
	{
		return false;
	}

	_assembly->image = mono_assembly_get_image(_assembly->assembly);
	if (!_assembly->image) {
		printf("Failed to get image");
		return false;
	}
	return true;
}

void loadClasses(ScriptAssembly* coreAsssembly, ScriptAssembly* assembly)
{
	MonoClass* baseScriptClass = mono_class_from_name(coreAsssembly->image, "Simengine", "BaseScript");
	auto classes = GetAssemblyClassList(assembly->image);
	for (auto& monoClass : classes)
	{
		bool isBaseScript = mono_class_is_subclass_of(monoClass, baseScriptClass, false);
		if (!isBaseScript)
			continue;

		auto object = mono_object_new(domain, monoClass);
		if (!object) {
			mono_image_close(assembly->image);
			printf("Failed to create class instance");
		}

		auto attributes = mono_custom_attrs_from_class(monoClass);
		if (attributes)
		{
			for (int i = 0; i < attributes->num_attrs; ++i)
			{
				auto attrClass = mono_method_get_class(attributes->attrs[i].ctor);
				const char* attrClassName = mono_class_get_name(attrClass);
				printf("Attribute class name: %s\n", attrClassName);
				if (strcmp(attrClassName, "MenuItem") == 0)
				{
					auto attrObj = mono_custom_attrs_get_attr(attributes, attrClass);
					auto nameProp = mono_class_get_field_from_name(attrClass, "Name");
					MonoString* strval;
					mono_field_get_value(attrObj, nameProp, &strval);

					printf("Menuu Attribute: %s\n", mono_string_to_utf8(strval));
				}
				if (strcmp(attrClassName, "EditorItem") == 0)
				{
					auto attrObj = mono_custom_attrs_get_attr(attributes, attrClass);
					auto nameProp = mono_class_get_field_from_name(attrClass, "editorName");
					MonoString* strval;
					mono_field_get_value(attrObj, nameProp, &strval);

					printf("EditorItem: %s\n", mono_string_to_utf8(strval));
				}
			}

		}

		mono_runtime_object_init(object);


		void* propIt = nullptr;
		auto property = mono_class_get_properties(monoClass, &propIt);
		while (property)
		{
			printf("Property : %s\n", mono_property_get_name(property));
			printf("Type : \n");
			printf("Access : \n");
			property = mono_class_get_properties(monoClass, &propIt);
		}


		void* filedIt = nullptr;
		auto field = mono_class_get_fields(monoClass, &filedIt);
		while (field)
		{
			FieldDef def = loadFieldDef(field);
			std::cout << def << std::endl;
			field = mono_class_get_fields(monoClass, &filedIt);
		}


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
}
