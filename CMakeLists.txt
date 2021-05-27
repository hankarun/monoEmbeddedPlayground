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

set(SOURCE_LIST
	src/Application.h
	src/Application.cpp
	src/ScriptApi.h
	src/ScriptHelper.h
	src/ScriptHelper.cpp
	src/ScriptInstance.h
	src/ScriptInstance.cpp
	src/ScriptFramework.cpp
	src/ScriptFramework.h
	src/ImGuiFrame.cpp
	src/ImGuiFrame.h
	src/main.cpp
	imgui/imconfig.h
	imgui/imgui.h
	imgui/imgui.cpp
	imgui/imgui_draw.cpp
	imgui/imgui_impl_dx11.h
	imgui/imgui_impl_dx11.cpp
	imgui/imgui_impl_win32.h
	imgui/imgui_impl_win32.cpp
	imgui/imgui_internal.h
	imgui/imgui_tables.cpp
	imgui/imgui_widgets.cpp
	imgui/imstb_rectpack.h
	imgui/imstb_textedit.h
	imgui/imstb_truetype.h
)

add_executable(monoScripting
	${SOURCE_LIST})

target_link_libraries(monoScripting libmono)
target_link_libraries(monoScripting d3d11.lib;)

source_group(TREE ${CMAKE_SOURCE_DIR} FILES ${SOURCE_LIST})
set_property(TARGET monoScripting PROPERTY VS_DEBUGGER_WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}")
