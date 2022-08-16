
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_library(libmono STATIC IMPORTED )
set_target_properties(libmono PROPERTIES
		IMPORTED_LOCATION_RELEASE ${CMAKE_CURRENT_SOURCE_DIR}/Mono_6.12.0.86/libmono-static-sgen.lib
        IMPORTED_LOCATION_DEBUG ${CMAKE_CURRENT_SOURCE_DIR}/Mono_6.12.0.86/libmono-static-sgen_debug.lib
        INTERFACE_INCLUDE_DIRECTORIES ${CMAKE_CURRENT_SOURCE_DIR}/Mono_6.12.0.86
)

set(SOURCE_FILES main.cc mono.cpp)
add_executable(raytracer_in_one_weekend ${SOURCE_FILES})
target_link_libraries(raytracer_in_one_weekend libmono Winmm Bcrypt version ws2_32)