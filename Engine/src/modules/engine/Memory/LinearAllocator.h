//
// Created by cmorg on 7/6/2026.
//

#pragma once

class LinearAllocator {
private:
    unsigned long totalSize = 0;
    unsigned long allocated = 0;
    void* block = nullptr;
    bool bOwnsMemory = false;

public:
    LinearAllocator() = default;
    ~LinearAllocator();
    void initialize(unsigned long size, void* memory);
    void shutdown();

    void* allocate(unsigned long size);
    void freeAll();
};