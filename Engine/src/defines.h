//Detects supported platforms

#pragma once
#include <iostream>
#include <string>

#define INVALID_ID_U32 4294967295U
#define INVALID_ID_U16 65535U
#define INVALID_ID_U8 255U

#define GIBIBYTES(amount) (amount * 1024 * 1024 * 1024)
#define MEBIBYTES(amount) (amount * 1024 * 1024)
#define KIBIBYTES(amount) (amount * 1024)
#define GIGABYTES(amount) (amount * 1000 * 1000 * 1000)
#define MEGABYTES(amount) (amount * 1000 * 1000)
#define KILOBYTES(amount) (amount * 1000)

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

using String = std::string;
using std::cout;
using std::endl;
using std::cerr;

struct MemoryRange {
    unsigned long offset = 0;
    unsigned long size = 0;
};

inline unsigned long alignMemory(const unsigned long operand, const unsigned long granularity) {
    return ((operand + (granularity - 1)) & ~(granularity - 1));
}

inline MemoryRange getAlignedRange(const unsigned long offset, const unsigned long size, const unsigned long granularity) {
    return MemoryRange{alignMemory(offset, granularity), alignMemory(size, granularity)};
}
