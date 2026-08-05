//
// Created by cmorg on 8/5/2026.
//

#include "FreeList.h"

#include "FF_Memory.h"

FreeList * FreeList::createFreeList(const unsigned long size, unsigned long memoryRequirement, void *memory) {
    if (!memory) return nullptr;

    const unsigned long entries = size / (sizeof(void*) * sizeof(FreeListNode));

    //Free lists have a min memory recommendation. Warn the use if they are below this threshold.
    constexpr unsigned long minMemory = (sizeof(FreeList) + sizeof(FreeListNode)) * 8;
    if (size < minMemory) {
        Logger::logWarn("FreeList detected the given memory block is smaller than " + std::to_string(minMemory) + ". Using a free list here is not recommended.");
    }

    FF_Memory::ff_clear(memory, memoryRequirement);

    const auto freeList = new (memory) FreeList();
    freeList->memoryBlock = memory;

    freeList->nodes = reinterpret_cast<FreeListNode*>(static_cast<unsigned char *>(freeList->memoryBlock)) + sizeof(FreeList);
    freeList->maxEntries = entries;
    freeList->totalSize = size;

    FF_Memory::ff_clear(freeList->nodes, sizeof(FreeListNode) * freeList->maxEntries);
    for (unsigned long i = 0; i < freeList->maxEntries; i++) {
        freeList->nodes[i].offset = 0;
        freeList->nodes[i].size = 0;
    }

    freeList->first = &freeList->nodes[0];
    freeList->first->offset = 0;
    freeList->first->size = size;
    freeList->first->next = nullptr;

    return freeList;
}

bool FreeList::free(const unsigned long size, const unsigned long offset) {
        if (memoryBlock == nullptr) return false;

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
    if (!memoryBlock) return;

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

void FreeList::shutdown() const {
    if (memoryBlock != nullptr) {
        void* memoryRef = memoryBlock;
        const unsigned int entriesRef = maxEntries;

        std::destroy_at(static_cast<FreeList *>(memoryRef));
        FF_Memory::ff_clear(memoryRef, sizeof(FreeList) + sizeof(FreeListNode) * entriesRef);
    }
}

bool FreeList::allocate(const unsigned long size, unsigned long &offset) {
    if (memoryBlock == nullptr) return false;

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
