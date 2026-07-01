set(SANDBOX_NAME FoxFire_Sandbox)

file(GLOB SOURCES src/modules/*/*/*.cpp)
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