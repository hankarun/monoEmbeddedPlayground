
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)


set(raylib_LIBRARY ${CMAKE_SOURCE_DIR}/raylib/lib/raylib.lib)
set(raylib_INCLUDE_DIR ${CMAKE_SOURCE_DIR}/raylib/include)
find_package(raylib REQUIRED)

add_library(libmono STATIC IMPORTED )
set_target_properties(libmono PROPERTIES
		IMPORTED_LOCATION_RELEASE ${CMAKE_CURRENT_SOURCE_DIR}/Mono_6.12.0.86/libmono-static-sgen.lib
        IMPORTED_LOCATION_DEBUG ${CMAKE_CURRENT_SOURCE_DIR}/Mono_6.12.0.86/libmono-static-sgen_debug.lib
        INTERFACE_INCLUDE_DIRECTORIES ${CMAKE_CURRENT_SOURCE_DIR}/Mono_6.12.0.86
)

set(SOURCE_FILES 
    main.cc 
    camera.h
    hitable.h
    hitable_list.h
    material.h
    mono.cpp
    mono.h
    random.h
    ray.h
    sphere.h
    triangle.h
    triangle.cpp
    vec3.h
)
add_executable(engine ${SOURCE_FILES})
target_link_libraries(engine libmono Winmm Bcrypt version ws2_32 raylib)
target_include_directories(engine PRIVATE ${raylib_INCLUDE_DIRS})