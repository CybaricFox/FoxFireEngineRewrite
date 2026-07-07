set(SANDBOX_NAME FoxFire_Sandbox)

file(GLOB_RECURSE SOURCES CONFIGURE_DEPENDS src/modules/*.cpp)
add_executable(${SANDBOX_NAME} src/Entry.cpp ${SOURCES})

add_library(FoxFire_Entry
        STATIC
        ../Engine/src/IEntry.cpp
)

target_link_libraries(FoxFire_Entry
        PUBLIC
        FoxFire_Engine
)

target_link_libraries(${SANDBOX_NAME}
        PRIVATE
        FoxFire_Entry
        FoxFire_Engine
)

add_custom_command(TARGET ${SANDBOX_NAME} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_directory
        ${CMAKE_SOURCE_DIR}/Assets
        $<TARGET_FILE_DIR:${SANDBOX_NAME}>/Assets
)

if(WIN32 AND MINGW)
    target_link_options(${SANDBOX_NAME}
            PRIVATE
            -static-libgcc
            -static-libstdc++
    )
endif()

set_target_properties(${SANDBOX_NAME} PROPERTIES
        RUNTIME_OUTPUT_DIRECTORY ${FOXFIRE_OUTPUT_DIR}
)

if(WIN32 AND MINGW)
    get_filename_component(MINGW_BIN_DIR "${CMAKE_CXX_COMPILER}" DIRECTORY)
    set(WINPTHREAD_DLL "${MINGW_BIN_DIR}/libwinpthread-1.dll")

    if(EXISTS "${WINPTHREAD_DLL}")
        add_custom_command(TARGET ${SANDBOX_NAME} POST_BUILD
                COMMAND ${CMAKE_COMMAND} -E copy_if_different
                "${WINPTHREAD_DLL}"
                "$<TARGET_FILE_DIR:${SANDBOX_NAME}>/libwinpthread-1.dll"
                COMMENT "Copying libwinpthread-1.dll beside ${SANDBOX_NAME}"
        )
    else()
        message(WARNING "libwinpthread-1.dll was not found at: ${WINPTHREAD_DLL}")
    endif()
endif()

# Build shaders
find_program(GLSL_VALIDATOR glslangValidator HINTS
        ${Vulkan_GLSLANG_VALIDATOR_EXECUTABLE}
        /usr/bin
        /usr/local/bin
        $ENV{VULKAN_SDK}/Bin/
        $ENV{VULKAN_SDK}/Bin32/
)

if (NOT GLSL_VALIDATOR)
    message(FATAL_ERROR "glslangValidator not found. Ensure Vulkan SDK is installed and on PATH.")
endif()

file(GLOB_RECURSE GLSL_SOURCE_FILES CONFIGURE_DEPENDS
        "${CMAKE_SOURCE_DIR}/Shaders/*.frag"
        "${CMAKE_SOURCE_DIR}/Shaders/*.vert"
)

set(SPIRV_BINARY_FILES "")

foreach(GLSL IN LISTS GLSL_SOURCE_FILES)
    get_filename_component(FILE_NAME "${GLSL}" NAME)

    set(SPIRV "${FOXFIRE_OUTPUT_DIR}/Shaders/${FILE_NAME}.spv")

    message(STATUS "Found shader: ${GLSL}")

    add_custom_command(
            OUTPUT "${SPIRV}"
            COMMAND ${CMAKE_COMMAND} -E make_directory
            "${FOXFIRE_OUTPUT_DIR}/Shaders"
            COMMAND "${GLSL_VALIDATOR}" -V "${GLSL}" -o "${SPIRV}"
            DEPENDS "${GLSL}"
            COMMENT "Compiling shader ${FILE_NAME}"
            VERBATIM
    )

    list(APPEND SPIRV_BINARY_FILES "${SPIRV}")
endforeach()

add_custom_target(SandboxShaders ALL
        DEPENDS ${SPIRV_BINARY_FILES}
)

add_dependencies(${SANDBOX_NAME} SandboxShaders)