//
// Created by cmorg on 7/1/2026.
//

#include "FF_Memory.h"

#include <cstring>
#include <iomanip>

#include "../Library/Logger.h"

MemoryBlock* FF_Memory::memoryData = nullptr;

String FF_Memory::getStringFromTag(const unsigned long tag) {
    switch (tag) {
        case 0: return "UNKNOWN";
        case 1: return "GAME";
        case 2: return "RENDER";
        case 3: return "ARRAY";
        case 4: return "LINEAR ALLOCATOR";
        case 5: return "DYNAMIC ARRAY";
        case 6: return "CHAR ARRAY";
        case 7: return "TEXTURE";
        case 8: return "HASHMAP";
        case 9: return "REUSABLE_ARRAY";
        default: return " ";
    }
}

//ff_set should set the memory block to the beginning, but just in case, REMEMBER TO ZERO MEMORY IN OWNER IF HEAP CORRUPTION OCCURS!!!
void FF_Memory::ff_free(void * block, const unsigned long size, const MemoryTag tag) {
    if (!block) return;

    if (tag == UNKNOWN) {
        Logger::logWarn("Free called with Unknown tag. Add a tag for this allocation!");
    }

    if (!memoryData) {
        cerr << "ff_free called after memoryData was destroyed!" << endl;
        return;
    }

    if (memoryData->taggedAllocations[tag] < size) {
        Logger::logError(
            "Memory underflow detected for tag " +
            std::string(getStringFromTag(tag)) +
            ". Current: " + std::to_string(memoryData->taggedAllocations[tag]) +
            ", freeing: " + std::to_string(size)
        );
        memoryData->totalAllocated -= memoryData->taggedAllocations[tag];
        memoryData->taggedAllocations[tag] = 0;
        free(block);
        return;
    }

    memoryData->totalAllocated -= size;
    memoryData->taggedAllocations[tag] -= size;
    free(block);
}

void * FF_Memory::ff_clear(void *block, const unsigned long size) {
    memset(block, 0, size);
    return block;
}

void * FF_Memory::ff_copy(void *destination, const void *source, const unsigned long size) {
    return memcpy(destination, source, size);
}

void * FF_Memory::ff_move(void *destination, const void *source, const unsigned long size) {
    return memmove(destination, source, size);
}

void * FF_Memory::ff_set(void *destination, const int value, const unsigned long size) {
    return memset(destination, value, size);
}

String FF_Memory::getMemoryUsage() {
    constexpr unsigned long gb = 1024 * 1024 * 1024;
    constexpr unsigned long mb = 1024 * 1024;
    constexpr unsigned long kb = 1024;

    const String title = "Tracked system memory usage (tagged):\n";
    String outString{};
    outString.append(title);

    for (unsigned int i = 0; i < MAX_TAGS; i++) {
        char unit[3] = "XB";
        float amount = 0.0f;
        if (memoryData->taggedAllocations[i] > gb) {
            unit[0] = 'G';
            amount = static_cast<float>(memoryData->taggedAllocations[i]) / static_cast<float>(gb);
        } else if (memoryData->taggedAllocations[i] > mb) {
            unit[0] = 'M';
            amount = static_cast<float>(memoryData->taggedAllocations[i]) / mb;
        } else if (memoryData->taggedAllocations[i] > kb) {
            unit[0] = 'K';
            amount = static_cast<float>(memoryData->taggedAllocations[i]) / kb;
        } else {
            unit[0] = 'B';
            unit[1] = 0;
            amount = static_cast<float>(memoryData->taggedAllocations[i]);
        }

        std::ostringstream oss;
        oss << getStringFromTag(i) << ": "<< std::fixed << std::setprecision(2) << amount << unit << "\n";
        outString.append(oss.str());
    }

    return outString;
}

void FF_Memory::initialize(MemoryBlock& memoryBlock) {
    memoryData = &memoryBlock;
    ff_clear(memoryData, sizeof(MemoryBlock));
}

void FF_Memory::shutdown() {
    memoryData = nullptr;
}

unsigned long FF_Memory::getAllocationCount() {
    if (memoryData) {
        return memoryData->allocationCount;
    }

    return 0;
}

void * FF_Memory::ff_allocate(const unsigned long size, const MemoryTag tag) {
    if (tag == UNKNOWN) {
        Logger::logWarn("Allocate called with Unknown tag. Add a tag for this allocation!");
    }

    if (memoryData) {
        memoryData->totalAllocated += size;
        memoryData->taggedAllocations[tag] += size;
        memoryData->allocationCount++;
    }

    void* block = malloc(size);
    ff_clear(block, size);
    return block;
}
