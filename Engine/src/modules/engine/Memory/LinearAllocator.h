//
// Created by cmorg on 7/6/2026.
//

#pragma once


class LinearAllocator {
private:
    unsigned long totalSize;
    unsigned long allocated;
    void* block;
    bool bOwnsMemory;

public:
    LinearAllocator(unsigned long size, void* memory);
    ~LinearAllocator();

    bool allocate(unsigned long size);
    void freeAll();
};