set(ENGINE_NAME FoxFire_Engine)

# Find and Set Vulkan Libraries
find_package(Vulkan REQUIRED)

include(GenerateExportHeader)

file(GLOB_RECURSE SOURCES CONFIGURE_DEPENDS src/modules/*.cpp)
add_library(${ENGINE_NAME} SHARED ${SOURCES})

generate_export_header(${ENGINE_NAME}
        BASE_NAME FOXFIRE
        EXPORT_MACRO_NAME FOXFIRE_API
        EXPORT_FILE_NAME ${CMAKE_CURRENT_BINARY_DIR}/foxfire_export.h
)

target_include_directories(${ENGINE_NAME}
        PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}
        ${CMAKE_CURRENT_BINARY_DIR}
)

if(WIN32 AND MINGW)
    target_link_options(${ENGINE_NAME}
            PRIVATE
            -static-libgcc
            -static-libstdc++
    )
elseif(UNIX)
    target_link_options(${ENGINE_NAME}
            PRIVATE
            -Wl,--no-undefined
    )
endif()

if(WIN32 AND MINGW)
    target_link_libraries(${ENGINE_NAME} Vulkan::Vulkan)
elseif(UNIX)
    find_package(PkgConfig REQUIRED)

    pkg_check_modules(X11 REQUIRED IMPORTED_TARGET x11)
    pkg_check_modules(X11_XCB REQUIRED IMPORTED_TARGET x11-xcb)
    pkg_check_modules(XCB REQUIRED IMPORTED_TARGET xcb)

    target_link_libraries(${ENGINE_NAME}
            PRIVATE
            PkgConfig::X11
            PkgConfig::X11_XCB
            PkgConfig::XCB
            Vulkan::Vulkan
    )

    target_link_options(${ENGINE_NAME}
            PRIVATE
            -Wl,--no-undefined
    )
endif()

set_target_properties(${ENGINE_NAME} PROPERTIES
        RUNTIME_OUTPUT_DIRECTORY ${FOXFIRE_OUTPUT_DIR}
        LIBRARY_OUTPUT_DIRECTORY ${FOXFIRE_OUTPUT_DIR}
        ARCHIVE_OUTPUT_DIRECTORY ${FOXFIRE_OUTPUT_DIR}

        RUNTIME_OUTPUT_DIRECTORY_DEBUG ${FOXFIRE_OUTPUT_DIR}
        LIBRARY_OUTPUT_DIRECTORY_DEBUG ${FOXFIRE_OUTPUT_DIR}
        ARCHIVE_OUTPUT_DIRECTORY_DEBUG ${FOXFIRE_OUTPUT_DIR}
)