cmake_minimum_required(VERSION 3.12)

project (monoScripting)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_library(libmono STATIC IMPORTED )
set_target_properties(libmono PROPERTIES
		IMPORTED_LOCATION_RELEASE ${CMAKE_SOURCE_DIR}/Mono_6.12.0.86/libmono-static-sgen.lib
        IMPORTED_LOCATION_DEBUG ${CMAKE_SOURCE_DIR}/Mono_6.12.0.86/libmono-static-sgen_debug.lib
        INTERFACE_INCLUDE_DIRECTORIES ${CMAKE_SOURCE_DIR}/Mono_6.12.0.86
)

add_executable(monoScripting
	src/Application.h
	src/Application.cpp
	src/ScriptApi.h
	src/ScriptHelper.h
	src/ScriptHelper.cpp
	src/ScriptInstance.h
	src/ScriptInstance.cpp
	src/ScriptFramework.cpp
	src/ScriptFramework.h
	src/main.cpp)

target_link_libraries(monoScripting libmono)

set_property(TARGET monoScripting PROPERTY VS_DEBUGGER_WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}")
