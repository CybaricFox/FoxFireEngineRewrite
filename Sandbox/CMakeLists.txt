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

# ---------------------------------------------------------------------------
# Assets
# ---------------------------------------------------------------------------

set(ASSET_SOURCE_DIR "${CMAKE_SOURCE_DIR}/Assets")
set(ASSET_OUTPUT_DIR "${FOXFIRE_OUTPUT_DIR}/Assets")

set(FONT_SOURCE_DIR "${ASSET_SOURCE_DIR}/Textures/Fonts")
set(FONT_OUTPUT_DIR "${ASSET_OUTPUT_DIR}/Textures/Fonts")

file(GLOB_RECURSE ASSET_FILES
        CONFIGURE_DEPENDS
        "${ASSET_SOURCE_DIR}/*"
)

set(COPIED_ASSET_FILES "")
set(COMPILED_FONT_FILES "")

# ---------------------------------------------------------------------------
# Normal assets
# ---------------------------------------------------------------------------

foreach(ASSET IN LISTS ASSET_FILES)

    file(RELATIVE_PATH RELATIVE_PATH
            "${ASSET_SOURCE_DIR}"
            "${ASSET}"
    )

    get_filename_component(EXTENSION "${ASSET}" EXT)
    string(TOLOWER "${EXTENSION}" EXTENSION)

    # Font source files are handled by the font compiler.
    if(EXTENSION STREQUAL ".ttf" OR
            EXTENSION STREQUAL ".foxfont")
        continue()
    endif()

    set(OUTPUT_FILE
            "${ASSET_OUTPUT_DIR}/${RELATIVE_PATH}"
    )

    get_filename_component(
            OUTPUT_DIRECTORY
            "${OUTPUT_FILE}"
            DIRECTORY
    )

    add_custom_command(
            OUTPUT "${OUTPUT_FILE}"

            COMMAND ${CMAKE_COMMAND} -E make_directory
            "${OUTPUT_DIRECTORY}"

            COMMAND ${CMAKE_COMMAND} -E copy_if_different
            "${ASSET}"
            "${OUTPUT_FILE}"

            DEPENDS "${ASSET}"

            COMMENT "Copying asset ${RELATIVE_PATH}"

            VERBATIM
    )

    list(APPEND COPIED_ASSET_FILES
            "${OUTPUT_FILE}"
    )

endforeach()

# ---------------------------------------------------------------------------
# Compile fonts
# ---------------------------------------------------------------------------

file(GLOB FONT_CONFIG_FILES
        CONFIGURE_DEPENDS
        "${FONT_SOURCE_DIR}/*.FoxFont"
        "${FONT_SOURCE_DIR}/*.foxfont"
)

foreach(FONT_CONFIG IN LISTS FONT_CONFIG_FILES)

    # OpenSans.FoxFont -> OpenSans
    get_filename_component(
            FONT_NAME
            "${FONT_CONFIG}"
            NAME_WE
    )

    set(FONT_TTF
            "${FONT_SOURCE_DIR}/${FONT_NAME}.ttf"
    )

    if(NOT EXISTS "${FONT_TTF}")
        message(FATAL_ERROR
                "Font '${FONT_NAME}' has a .FoxFont file but no matching .ttf file."
        )
    endif()

    set(FONT_OUTPUT_CONFIG
            "${FONT_OUTPUT_DIR}/${FONT_NAME}.FoxFont"
    )

    set(FONT_OUTPUT_PNG
            "${FONT_OUTPUT_DIR}/${FONT_NAME}.png"
    )

    add_custom_command(
            OUTPUT
            "${FONT_OUTPUT_CONFIG}"
            "${FONT_OUTPUT_PNG}"

            COMMAND ${CMAKE_COMMAND} -E make_directory
            "${FONT_OUTPUT_DIR}"

            COMMAND $<TARGET_FILE:FoxFire_Tools>
            "Compile Font"
            "${FONT_SOURCE_DIR}/${FONT_NAME}"
            "${FONT_OUTPUT_DIR}"

            WORKING_DIRECTORY
            "${FONT_SOURCE_DIR}"

            DEPENDS
            FoxFire_Tools
            "${FONT_TTF}"
            "${FONT_CONFIG}"

            COMMENT
            "Compiling font ${FONT_NAME}"

            VERBATIM
    )

    list(APPEND COMPILED_FONT_FILES
            "${FONT_OUTPUT_CONFIG}"
            "${FONT_OUTPUT_PNG}"
    )

endforeach()

# ---------------------------------------------------------------------------
# Asset build targets
# ---------------------------------------------------------------------------

add_custom_target(CopyAssets
        DEPENDS ${COPIED_ASSET_FILES}
)

add_custom_target(CompileFonts
        DEPENDS ${COMPILED_FONT_FILES}
)

add_dependencies(${SANDBOX_NAME}
        SandboxShaders
        CopyAssets
        CompileFonts
)