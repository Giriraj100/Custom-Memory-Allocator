#pragma once

#include <cstddef>

#ifdef _WIN32
#include <windows.h>
#else
#include <mutex>
#endif

struct BlockHeader {
    size_t size;
    bool isFree;
    BlockHeader* next;
    BlockHeader* prev;
};

class MemoryAllocator {
public:
    MemoryAllocator(size_t poolSize);
    ~MemoryAllocator();

    void* alloc(size_t size);
    void free(void* ptr);

    // Prevent copying
    MemoryAllocator(const MemoryAllocator&) = delete;
    MemoryAllocator& operator=(const MemoryAllocator&) = delete;

private:
    void* pool;
    size_t poolSize;
    BlockHeader* freeList;
    
#ifdef _WIN32
    CRITICAL_SECTION cs;
#else
    std::mutex mtx;
#endif

    void coalesce();
    size_t align(size_t size);
};
