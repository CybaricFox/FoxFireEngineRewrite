//
// Created by cmorg on 7/1/2026.
//

#include "FF_Memory.h"

#include <cstring>
#include <format>

#include "Logger.h"

FF_Memory::MemoryBlock FF_Memory::memoryData{};

String FF_Memory::getStringFromTag(const unsigned long tag) {
    switch (tag) {
        case 0: return "UNKNOWN";
        case 1: return "GAME";
        default: return " ";
    }
}

FF_Memory::FF_Memory()
{
    ff_clear(&memoryData, sizeof(memoryData));
}

FF_Memory::~FF_Memory() {

}

void FF_Memory::ff_free(void *block, const unsigned long size, const MemoryTag tag) {
    if (tag == UNKNOWN) {
        Logger::logWarn("Free called with Unknown tag. Add a tag for this allocation!");
    }

    memoryData.totalAllocated -= size;
    memoryData.taggedAllocations[tag] -= size;

    free(block);
}

void * FF_Memory::ff_clear(void *block, const unsigned long size) {
    memset(block, 0, size);
    return block;
}

void * FF_Memory::ff_copy(void *destination, const void *source, const unsigned long size) {
    return memcpy(destination, source, size);
}

void * FF_Memory::ff_set(void *destination, const int value, const unsigned long size) {
    return memset(destination, value, size);
}

String FF_Memory::getMemoryUsage() {
    constexpr unsigned long gb = 1024 * 1024 * 1024;
    constexpr unsigned long mb = 1024 * 1024;
    constexpr unsigned long kb = 1024;

    const String title = "System memory usage (tagged):\n";
    String outString{};
    outString.append(title);

    for (unsigned int i = 0; i < MAX_TAGS; i++) {
        char unit[3] = "XB";
        float amount = 1;
        if (memoryData.taggedAllocations[i] > gb) {
            unit[0] = 'G';
            amount = memoryData.taggedAllocations[i] / gb;
        } else if (memoryData.taggedAllocations[i] > mb) {
            unit[0] = 'M';
            amount = memoryData.taggedAllocations[i] / mb;
        } else if (memoryData.taggedAllocations[i] > kb) {
            unit[0] = 'K';
            amount = memoryData.taggedAllocations[i] / kb;
        } else {
            unit[0] = 'B';
            unit[1] = 0;
            amount = memoryData.taggedAllocations[i];
        }

        outString.append(std::format("{}: {:.2f}{}\n", getStringFromTag(i), amount, unit));
    }

    return outString;
}

void * FF_Memory::ff_allocate(const unsigned long size, const MemoryTag tag) {
    if (tag == UNKNOWN) {
        Logger::logWarn("Allocate called with Unknown tag. Add a tag for this allocation!");
    }

    memoryData.totalAllocated += size;
    memoryData.taggedAllocations[tag] += size;

    void* block = malloc(size);
    ff_clear(block, size);
    return block;
}
