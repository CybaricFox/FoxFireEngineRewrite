//
// Created by cmorg on 7/1/2026.
//

#include "FF_Memory.h"

#include <cstring>
#include <iomanip>

#include "../Library/Logger.h"
#include "../Core/Platform.h"

FF_Memory* FF_Memory::memorySystem = nullptr;

String FF_Memory::getStringFromTag(const unsigned long tag) {
    switch (tag) {
        case 0: return "UNKNOWN";
        case 1: return "GAME";
        case 2: return "RENDER";
        case 3: return "ARRAY";
        case 4: return "LINEAR ALLOCATOR";
        case 5: return "DYNAMIC ARRAY";
        case 6: return "RESOURCE";
        case 7: return "TEXTURE";
        case 8: return "HASHMAP";
        case 9: return "REUSABLE_ARRAY";
        case 10: return "MATERIAL";
        default: return " ";
    }
}

//ff_set should set the memory block to the beginning, but just in case, REMEMBER TO ZERO MEMORY IN OWNER IF HEAP CORRUPTION OCCURS!!!
void FF_Memory::ff_free(void * block, const unsigned long size, const MemoryTag tag) {
    if (!block) return;

    if (tag == UNKNOWN) {
        Logger::logWarn("Free called with Unknown tag. Add a tag for this allocation!");
    }

    if (!memorySystem) {
        cerr << "ff_free called after memorySystem was destroyed!" << endl;
        Platform::platform_free(block, false);
    }

    if (memorySystem->memoryData.taggedAllocations[tag] < size) {
        Logger::logError(
            "Memory underflow detected for tag " +
            std::string(getStringFromTag(tag)) +
            ". Current: " + std::to_string(memorySystem->memoryData.taggedAllocations[tag]) +
            ", freeing: " + std::to_string(size)
        );
        memorySystem->memoryData.totalAllocated -= memorySystem->memoryData.taggedAllocations[tag];
        memorySystem->memoryData.taggedAllocations[tag] = 0;
        memorySystem->allocationCount--;
        Platform::platform_free(block, false);
        return;
    }

    memorySystem->memoryData.totalAllocated -= size;
    memorySystem->memoryData.taggedAllocations[tag] -= size;
    memorySystem->allocationCount--;

    if (!memorySystem->allocator.free(block, size)) {
        Platform::platform_free(block, false);
    }
}

void * FF_Memory::ff_clear(void *block, const unsigned long size) {
    Platform::platform_clear(block, size);
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
        if (memorySystem->memoryData.taggedAllocations[i] > gb) {
            unit[0] = 'G';
            amount = static_cast<float>(memorySystem->memoryData.taggedAllocations[i]) / static_cast<float>(gb);
        } else if (memorySystem->memoryData.taggedAllocations[i] > mb) {
            unit[0] = 'M';
            amount = static_cast<float>(memorySystem->memoryData.taggedAllocations[i]) / mb;
        } else if (memorySystem->memoryData.taggedAllocations[i] > kb) {
            unit[0] = 'K';
            amount = static_cast<float>(memorySystem->memoryData.taggedAllocations[i]) / kb;
        } else {
            unit[0] = 'B';
            unit[1] = 0;
            amount = static_cast<float>(memorySystem->memoryData.taggedAllocations[i]);
        }

        std::ostringstream oss;
        oss << getStringFromTag(i) << ": "<< std::fixed << std::setprecision(2) << amount << unit << "\n";
        outString.append(oss.str());
    }

    return outString;
}

bool FF_Memory::initialize(const MemoryConfig config) {
    constexpr unsigned long systemMemoryRequirement = sizeof(FF_Memory);
    const unsigned long allocationRequirement = DynamicAllocator::getMemoryRequirement(config.totalAllocationSize);

    void* memory = Platform::platform_allocate(systemMemoryRequirement + allocationRequirement, false);
    if (!memory) {
        Logger::logFatal("Memory system failed to allocate.");
        return false;
    }

    memorySystem = new (memory) FF_Memory();
    memorySystem->config = config;
    memorySystem->allocationCount = 0;
    memorySystem->allocationMemoryRequirement = allocationRequirement;
    memorySystem->memoryData = MemoryStats{};

    //Initialize the dynamic allocator
    void* allocatorMemory = static_cast<unsigned char *>(memory) + systemMemoryRequirement;
    memorySystem->allocator.initialize(config.totalAllocationSize, allocatorMemory);

    Logger::logDebug("Memory system allocated successfully with " + std::to_string(config.totalAllocationSize) + " bytes.");
    return true;
}

void FF_Memory::shutdown() {
    if (memorySystem) {
        memorySystem->allocator.shutdown();
        std::destroy_at(memorySystem);
        Platform::platform_free(memorySystem, false);
        memorySystem = nullptr;
    }
}

unsigned long FF_Memory::getAllocationCount() {
    if (memorySystem) {
        return memorySystem->allocationCount;
    }

    return 0;
}

void FF_Memory::trackEngineMemory(const unsigned long size) {
    if (memorySystem) {
        memorySystem->memoryData.totalAllocated += size;
        memorySystem->memoryData.taggedAllocations[GAME] += size;
        memorySystem->allocationCount++;
    }
}

void FF_Memory::untrackEngineMemory(const unsigned long size) {
    if (memorySystem) {
        memorySystem->memoryData.totalAllocated -= size;
        memorySystem->memoryData.taggedAllocations[GAME] -= size;
        memorySystem->allocationCount--;
    }
}

void * FF_Memory::ff_allocate(const unsigned long size, const MemoryTag tag) {
    if (tag == UNKNOWN) {
        Logger::logWarn("Allocate called with Unknown tag. Add a tag for this allocation!");
    }

    void* memory = nullptr;
    if (memorySystem) {
        memorySystem->memoryData.totalAllocated += size;
        memorySystem->memoryData.taggedAllocations[tag] += size;
        memorySystem->allocationCount++;
        memory = memorySystem->allocator.allocate(size);
    } else {
        Logger::logError("Allocate called before memory system is initialized!");
        memory = Platform::platform_allocate(size, false);
    }

    if (memory) {
        Platform::platform_clear(memory, size);
        return memory;
    }

    Logger::logFatal("FF_Allocate failed to allocate memory!");
    return nullptr;
}
