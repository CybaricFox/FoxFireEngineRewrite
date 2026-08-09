//
// Created by cmorg on 8/5/2026.
//

#include "FreeList.h"

#include "FF_Memory.h"

void FreeList::initialize(const unsigned long size, unsigned long memoryRequirement, void *memory) {
    if (!memory) return;

    //This may become a point of issues
    unsigned long entries = size / (sizeof(void*) * sizeof(FreeListNode));
    if (entries == 0) entries = 1;

    //Free lists have a min memory recommendation. Warn the use if they are below this threshold.
    constexpr unsigned long minMemory = sizeof(FreeListNode) * 8;
    if (size < minMemory) {
        Logger::logWarn("FreeList detected the given memory block is smaller than " + std::to_string(minMemory) + ". Using a free list here is not recommended.");
    }

    FF_Memory::ff_clear(memory, memoryRequirement);

    nodes = reinterpret_cast<FreeListNode*>(static_cast<unsigned char*>(memory));
    maxEntries = entries;
    totalSize = size;

    FF_Memory::ff_clear(nodes, sizeof(FreeListNode) * maxEntries);
    for (unsigned long i = 0; i < maxEntries; i++) {
        nodes[i].offset = 0;
        nodes[i].size = 0;
    }

    first = &nodes[0];
    first->offset = 0;
    first->size = size;
    first->next = nullptr;
}

bool FreeList::free(const unsigned long size, const unsigned long offset) {
        if (nodes == nullptr) return false;

        FreeListNode* node = first;
        FreeListNode* previous = nullptr;

        //Check that the entire list isn't allocated
        if (!node) {
            FreeListNode* newNode = getNode();
            newNode->offset = offset;
            newNode->size = size;
            newNode->next = nullptr;
            first = newNode;
            return true;
        }

        while (node) {
            if (node->offset + node->size == offset) {
                //append to this node
                node->size += size;

                //Check if the previous and next nodes are adjacent to this one and then combine them if they are
                if (node->next && node->next->offset == node->offset + node->size) {
                    node->size += node->next->size;
                    FreeListNode* next = node->next;
                    node->next = node->next->next;
                    freeNode(*next);
                }

                return true;
            }
            if (node->offset == offset) {
                Logger::logFatal("Double Free detected in Free List! Node Offset: " + std::to_string(node->offset));
                return false;
            }
            if (node->offset > offset) {
                //Create a new node
                FreeListNode* newNode = getNode();
                newNode->offset = offset;
                newNode->size = size;

                //If there is a previous node, insert it between this node and the previous one
                if (previous) {
                    previous->next = newNode;
                    newNode->next = node;
                } else {
                    //Otherwise, this node is the head
                    newNode->next = node;
                    first = newNode;
                }

                //Check if next node can be conjoined
                if (newNode->next && newNode->offset + newNode->size == newNode->next->offset) {
                    newNode->size += newNode->next->size;
                    FreeListNode* toFree = newNode->next;
                    newNode->next = toFree->next;
                    freeNode(*toFree);
                }
                //Check if previous node can be conjoined
                if (previous && previous->offset + previous->size == newNode->offset) {
                    previous->size += newNode->size;
                    FreeListNode* toFree = newNode;
                    previous->next = toFree->next;
                    freeNode(*toFree);
                }

                return true;
            }

            if (!node->next && node->offset + node->size < offset) {
                FreeListNode* newNode = getNode();
                newNode->offset = offset;
                newNode->size = size;
                newNode->next = nullptr;
                node->next = newNode;

                return true;
            }

            previous = node;
            node = node->next;
        }

        Logger::logWarn("FreeList was unable to find a memory block to be freed. Memory may be corrupted!");
        return false;
}

void FreeList::clear() const {
    if (!nodes) return;

    //Remember i = 0 is the FreeList itself!
    for (unsigned int i = 1; i < maxEntries; i++) {
        nodes[i].offset = 0;
        nodes[i].size = 0;
    }

    first->offset = 0;
    first->size = totalSize;
    first->next = nullptr;
}

void FreeList::freeNode(FreeListNode &node) {
    node.offset = 0;
    node.size = 0;
    node.next = nullptr;
}

//This function needs testing
bool FreeList::resize(void *newMemory, const unsigned long newSize, void*& outOldMemory) {
    if (!newMemory) return false;
    if (totalSize > newSize) return false;

    const unsigned long sizeDif = newSize - totalSize;
    unsigned long requiredNodes = 0;

    FreeListNode* oldNodes = nodes;
    FreeListNode* oldFirst = first;
    unsigned long oldMaxEntries = maxEntries;

    outOldMemory = oldNodes;

    unsigned long newMemoryRequirement = calculateMemoryRequirement(newSize);
    unsigned long newMaxEntries = newMemoryRequirement / sizeof(FreeListNode);

    for (const FreeListNode* node = oldFirst; node != nullptr; node = node->next) {
        requiredNodes++;
    }

    if (sizeDif > 0) {
        if (!oldFirst) {
            requiredNodes++;
        } else {
            const FreeListNode* last = oldFirst;

            while (last->next) {
                last = last->next;
            }

            // If the last free block doesn't touch the old end,
            // resizing creates another free block.
            if (last->offset + last->size != totalSize) {
                requiredNodes++;
            }
        }
    }

    if (requiredNodes > newMaxEntries) {
        Logger::logError("FreeList resize does not have enough node entries.");
        return false;
    }

    // Initialize the NEW node storage.
    FF_Memory::ff_clear(newMemory,sizeof(FreeListNode) * newMaxEntries);

    auto* newNodes = static_cast<FreeListNode*>(newMemory);

    FreeListNode* newFirst = nullptr;
    FreeListNode* previous = nullptr;

    unsigned long nodeIndex = 0;

    // Copy existing free blocks.
    for (const FreeListNode* oldNode = oldFirst; oldNode != nullptr; oldNode = oldNode->next) {
        FreeListNode* newNode = &newNodes[nodeIndex++];

        newNode->offset = oldNode->offset;
        newNode->size = oldNode->size;
        newNode->next = nullptr;

        if (!newFirst) {
            newFirst = newNode;
        }

        if (previous) {
            previous->next = newNode;
        }

        previous = newNode;
    }

    // Add the newly available memory.
    if (sizeDif > 0) {
        if (!previous) {
            // The old free list was completely empty.
            FreeListNode* newNode = &newNodes[nodeIndex];

            newNode->offset = totalSize;
            newNode->size = sizeDif;
            newNode->next = nullptr;

            newFirst = newNode;
        } else if (previous->offset + previous->size == totalSize) {
            // Existing last free block touches the end.
            previous->size += sizeDif;
        } else {
            // Need a new free block at the end.
            FreeListNode* newNode = &newNodes[nodeIndex];

            newNode->offset = totalSize;
            newNode->size = sizeDif;
            newNode->next = nullptr;

            previous->next = newNode;
        }
    }

    // Commit the new FreeList state.
    nodes = newNodes;
    first = newFirst;
    maxEntries = newMaxEntries;
    totalSize = newSize;

    return true;
}

void FreeList::shutdown() {
    if (nodes != nullptr) {
        FF_Memory::ff_clear(nodes, sizeof(FreeListNode) * maxEntries);
    }

    nodes = nullptr;
    first = nullptr;
    maxEntries = 0;
    totalSize = 0;
}

bool FreeList::allocate(const unsigned long size, unsigned long &offset) {
    if (nodes == nullptr) return false;

    FreeListNode* node = first;
    FreeListNode* previous = nullptr;

    while (node) {
        //We found an exact match
        if (node->size == size) {
            offset = node->offset;
            FreeListNode* result = nullptr;
            if (previous) {
                previous->next = node->next;
                result = node;
            } else {
                result = first;
                first = node->next;
            }

            freeNode(*result);
            return true;
        }
        //Free space is larger than the request, use it.
        if (node->size > size) {
            offset = node->offset;
            node->size -= size;
            node->offset += size;
            return true;
        }

        previous = node;
        node = node->next;
    }

    //Not enough space!
    Logger::logWarn("FreeList could not find a memory block with enough free space. Requested: " + std::to_string(size) + " Available: " + std::to_string(getFreeSpace()));
    return false;
}
