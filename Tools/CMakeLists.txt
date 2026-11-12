add_executable(FoxFire_Tools Main.cpp
        FileHandler.cpp
        FontCompiler.cpp
        StringUtils.cpp
)

message(STATUS "CMAKE_SOURCE_DIR = ${CMAKE_SOURCE_DIR}")

target_include_directories(FoxFire_Tools PRIVATE
        "${CMAKE_CURRENT_SOURCE_DIR}"
        "${CMAKE_SOURCE_DIR}/Dependencies/stb"
)

# Keep the tool executable inside the Tools directory.
set_target_properties(FoxFire_Tools PROPERTIES
        RUNTIME_OUTPUT_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}"
)

# Prevent multi-config generators such as Visual Studio from
# putting the executable in Tools/Debug or Tools/Release.
set_target_properties(FoxFire_Tools PROPERTIES
        RUNTIME_OUTPUT_DIRECTORY_DEBUG "${CMAKE_CURRENT_SOURCE_DIR}"
        RUNTIME_OUTPUT_DIRECTORY_RELEASE "${CMAKE_CURRENT_SOURCE_DIR}"
        RUNTIME_OUTPUT_DIRECTORY_RELWITHDEBINFO "${CMAKE_CURRENT_SOURCE_DIR}"
        RUNTIME_OUTPUT_DIRECTORY_MINSIZEREL "${CMAKE_CURRENT_SOURCE_DIR}"
)