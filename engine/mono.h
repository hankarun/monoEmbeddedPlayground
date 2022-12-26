#include <string>
#include "Path.h"
#include <unordered_map>

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
#include <mono/metadata/tabledefs.h>

enum class ScriptFieldType
{
	None = 0,
	Float, Double,
	Bool, Char, Byte, Short, Int, Long,
	UByte, UShort, UInt, ULong,
	Vector2, Vector3, Vector4,
	Entity, Prefab, Texture
};

static std::unordered_map<std::string, ScriptFieldType> s_ScriptFieldTypeMap =
{
	{ "System.Single", ScriptFieldType::Float },
	{ "System.Double", ScriptFieldType::Double },
	{ "System.Boolean", ScriptFieldType::Bool },
	{ "System.Char", ScriptFieldType::Char },
	{ "System.Int16", ScriptFieldType::Short },
	{ "System.Int32", ScriptFieldType::Int },
	{ "System.Int64", ScriptFieldType::Long },
	{ "System.Byte", ScriptFieldType::Byte },
	{ "System.UInt16", ScriptFieldType::UShort },
	{ "System.UInt32", ScriptFieldType::UInt },
	{ "System.UInt64", ScriptFieldType::ULong },

	{ "Simengine.Vector2", ScriptFieldType::Vector2 },
	{ "Simengine.Vector3", ScriptFieldType::Vector3 },
	{ "Simengine.Vector4", ScriptFieldType::Vector4 },

	{ "Simengine.Entity", ScriptFieldType::Entity },
	{ "Simengine.Prefab", ScriptFieldType::Prefab },
	{ "Simengine.Texture", ScriptFieldType::Texture }
};

bool initialize(const Path& directory);
struct ScriptAssembly
{
	MonoAssembly* assembly;
	MonoImage* image;
};



bool loadAssembly(ScriptAssembly* _assembly, const Path& path);
void loadClasses(ScriptAssembly* coreAsssembly, ScriptAssembly* assembly);