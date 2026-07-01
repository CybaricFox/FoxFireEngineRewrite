//Detects supported platforms

#pragma once

//Windows
#if defined(WIN32) || defined(_WIN32) || defined(__WIN32__)
    #define FOXFIRE_PLATFORM_WINDOWS 1
    #ifndef _WIN64
        #error "64-bit is required on Windows"
    #endif

//Linux
#elif defined(__linux__) || defined(__gnu_linux__)
    #define FOXFIRE_PLATFORM_LINUX 1

//Unix
#elif defined(__unix__)
    #define FOXFIRE_PLATFORM_UNIX 1

//Unsupported Platform
#else
    #error "This platform is not supported!"
#endif
