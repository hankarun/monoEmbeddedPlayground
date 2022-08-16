
add_library(Game SHARED
	TestScript.cs
)

target_link_libraries(Game PRIVATE EngineApi)