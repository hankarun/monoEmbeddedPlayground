#pragma once
#include <mono/metadata/assembly.h>
#include <mono/metadata/attrdefs.h>
#include <mono/metadata/debug-helpers.h>
#include <mono/metadata/environment.h>
#include <mono/metadata/mono-config.h>
#include <mono/metadata/object.h>

#include <string>

class ScriptInstance {
public:
    template <class T>
    bool SetValue(T* value, const std::string& name)
    {
        if (MonoClassField* field = mono_class_get_field_from_name(klass, name.c_str())) {
            mono_field_set_value(object, field, value);
            return true;
        }

        printf("Failed to set value for field %s", name.c_str());
        return false;
    }

    bool SetStringValue(MonoDomain* domain, std::string* value, const std::string& name)
    {
        if (MonoClassField* field = mono_class_get_field_from_name(klass, name.c_str())) {
            MonoString * str = mono_string_new(domain, value->c_str());
            mono_field_set_value(object, field, str);
            return true;
        }

        printf("Failed to set value for field %s", name.c_str());
        return false;
    }

    static ScriptInstance load(MonoDomain* domain, const std::string& file_path);
    void serializeData(const std::string& json_path) const;
    void deserializeData(MonoDomain* domain, const std::string& json_path);

    void printFields();
    void output_methods();
    void output_properties();

    void init();
    void update();

private:
    MonoAssembly* assembly = nullptr;
    MonoImage* image = nullptr;
    MonoClass* klass = nullptr;
    MonoObject* object = nullptr;
    MonoMethod* method_start = nullptr;
    MonoMethod* method_update = nullptr;
};