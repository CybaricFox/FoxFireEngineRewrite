set(ENGINE_NAME FoxFire_Engine)

# Find and Set Vulkan Libraries
find_package(Vulkan REQUIRED)

include(GenerateExportHeader)

file(GLOB SOURCES src/modules/*/*/*.cpp)
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

target_link_options(${ENGINE_NAME}
        PRIVATE
        -static-libgcc
        -static-libstdc++
)

target_link_libraries(${ENGINE_NAME} Vulkan::Vulkan)

set_target_properties(${ENGINE_NAME} PROPERTIES
        RUNTIME_OUTPUT_DIRECTORY ${FOXFIRE_OUTPUT_DIR}
        LIBRARY_OUTPUT_DIRECTORY ${FOXFIRE_OUTPUT_DIR}
        ARCHIVE_OUTPUT_DIRECTORY ${FOXFIRE_OUTPUT_DIR}

        RUNTIME_OUTPUT_DIRECTORY_DEBUG ${FOXFIRE_OUTPUT_DIR}
        LIBRARY_OUTPUT_DIRECTORY_DEBUG ${FOXFIRE_OUTPUT_DIR}
        ARCHIVE_OUTPUT_DIRECTORY_DEBUG ${FOXFIRE_OUTPUT_DIR}
)