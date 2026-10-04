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
        case 11: return "ECS";
        case 12: return "RING_QUEUE";
        case 13: return "JOB";
        case 14: return "RENDER_BACKEND_MANUAL";
        case 15: return "RENDER_BACKEND_AUTO";
        case 16: return "GPU";
        default: return " ";
    }
}

String FF_Memory::getUnitForSize(const ULong size, float &outAmount) {
    if (size >= GIBIBYTES(1)) {
        outAmount = static_cast<float>(static_cast<double>(size) / GIBIBYTES(1));
        return "GiB";
    }

    if (size >= MEBIBYTES(1)) {
        outAmount = static_cast<float>(static_cast<double>(size) / MEBIBYTES(1));
        return "MiB";
    }

    if (size >= KIBIBYTES(1)) {
        outAmount = static_cast<float>(static_cast<double>(size) / KIBIBYTES(1));
        return "KiB";
    }

    outAmount = static_cast<float>(size);
    return "B";
}

bool FF_Memory::getSizeAndAlignment(void *memory, ULong &size, unsigned short &alignment) {
    return allocator.getSizeAndAlignment(memory, size, alignment);
}

void FF_Memory::reportAllocation(const ULong size, const MemoryTag tag) {
    if (!Platform::lockMutex(allocationMutex)) {
        Logger::logFatal("Failed to lock allocation mutex while reporting an allocation.");
        return;
    }
    memorySystem->memoryData.totalAllocated += size;
    memorySystem->memoryData.taggedAllocations[tag] += size;
    memorySystem->allocationCount++;
    if (!Platform::unlockMutex(allocationMutex)) {
        Logger::logFatal("Failed to unlock allocation mutex while reporting an allocation.");
    }
}

void FF_Memory::removeReport(const ULong size, const MemoryTag tag) {
    if (!Platform::lockMutex(allocationMutex)) {
        Logger::logFatal("Failed to lock allocation mutex while removing a report.");
        return;
    }
    memorySystem->memoryData.totalAllocated -= size;
    memorySystem->memoryData.taggedAllocations[tag] -= size;
    memorySystem->allocationCount--;
    if (!Platform::unlockMutex(allocationMutex)) {
        Logger::logFatal("Failed to unlock allocation mutex while removing a report.");
    }
}

//ff_set should set the memory block to the beginning, but just in case, REMEMBER TO ZERO MEMORY IN OWNER IF HEAP CORRUPTION OCCURS!!!
void FF_Memory::ff_free(void *block, const unsigned long size, const MemoryTag tag, unsigned short alignment) {
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

    if (!Platform::lockMutex(memorySystem->allocationMutex)) {
        Logger::logFatal("Memory system failed to lock mutex during free.");
        return;
    }

    memorySystem->memoryData.totalAllocated -= size;
    memorySystem->memoryData.taggedAllocations[tag] -= size;
    memorySystem->allocationCount--;

    if (!memorySystem->allocator.free(block)) {
        Platform::platform_free(block, false);
    }

    Platform::unlockMutex(memorySystem->allocationMutex);
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
    const String title = "Tracked system memory usage (tagged):\n";
    String outString{};
    outString.append(title);

    for (unsigned int i = 0; i < MAX_TAGS; i++) {
        float amount = 1;
        String unit = getUnitForSize(memorySystem->memoryData.taggedAllocations[i], amount);

        std::ostringstream oss;
        oss << getStringFromTag(i) << ": "<< std::fixed << std::setprecision(2) << amount << unit << "\n";
        outString.append(oss.str());
    }

    ULong totalSpace = memorySystem->allocator.getTotalSpace();
    ULong freeSpace = memorySystem->allocator.getFreeSpace();
    ULong usedSpace = totalSpace - freeSpace;

    float usedAmount = 1;
    String usedUnit = getUnitForSize(usedSpace, usedAmount);

    float totalAmount = 1;
    String totalUnit = getUnitForSize(totalSpace, totalAmount);

    double percentUsed = static_cast<double>(usedSpace) / static_cast<double>(totalSpace);

    outString.append("Overall Memory Usage: \n");

    std::ostringstream oss;
    oss << std::fixed << std::setprecision(2) << usedAmount << usedUnit << " Used.\n"
    << std::fixed << std::setprecision(2) << totalAmount << totalUnit << " Total.\n"
    << std::fixed << std::setprecision(2) << percentUsed << "% Used.";

    outString.append(oss.str());

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

    //Setup mutex for memory system
    if (!Platform::createMutex(memorySystem->allocationMutex)) {
        Logger::logFatal("Memory system failed to create mutex.");
        return false;
    }

    Logger::logDebug("Memory system allocated successfully with " + std::to_string(config.totalAllocationSize) + " bytes.");
    return true;
}

void FF_Memory::shutdown() {
    if (memorySystem) {
        Platform::destroyMutex(memorySystem->allocationMutex);

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

void * FF_Memory::ff_allocate(const unsigned long size, const MemoryTag tag, unsigned short alignment) {
    if (tag == UNKNOWN) {
        Logger::logWarn("Allocate called with Unknown tag. Add a tag for this allocation!");
    }

    void* memory = nullptr;
    if (memorySystem) {
        if (!Platform::lockMutex(memorySystem->allocationMutex)) {
            Logger::logFatal("Memory system failed to lock mutex during allocation.");
            return nullptr;
        }
        memorySystem->memoryData.totalAllocated += size;
        memorySystem->memoryData.taggedAllocations[tag] += size;
        memorySystem->allocationCount++;
        memory = memorySystem->allocator.allocate(size, alignment);

        Platform::unlockMutex(memorySystem->allocationMutex);
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
