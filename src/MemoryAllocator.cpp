#include "MemoryAllocator.hpp"
#include <cstdlib>
#include <iostream>

MemoryAllocator::MemoryAllocator(size_t poolSize) : poolSize(poolSize) {
    pool = std::malloc(poolSize);
    if (!pool) {
        throw std::bad_alloc();
    }

    freeList = static_cast<BlockHeader*>(pool);
    freeList->size = poolSize - sizeof(BlockHeader);
    freeList->isFree = true;
    freeList->next = nullptr;
    freeList->prev = nullptr;

#ifdef _WIN32
    InitializeCriticalSection(&cs);
#endif
}

MemoryAllocator::~MemoryAllocator() {
#ifdef _WIN32
    DeleteCriticalSection(&cs);
#endif
    std::free(pool);
}

size_t MemoryAllocator::align(size_t size) {
    return (size + 7) & ~7;
}

void* MemoryAllocator::alloc(size_t size) {
#ifdef _WIN32
    EnterCriticalSection(&cs);
#else
    std::lock_guard<std::mutex> lock(mtx);
#endif

    size = align(size);

    BlockHeader* current = freeList;
    while (current) {
        if (current->isFree && current->size >= size) {
            // Can we split the block?
            // We need enough space for header + some data (8 bytes)
            if (current->size >= size + sizeof(BlockHeader) + 8) {
                BlockHeader* nextBlock = reinterpret_cast<BlockHeader*>(
                    reinterpret_cast<char*>(current) + sizeof(BlockHeader) + size
                );
                nextBlock->size = current->size - size - sizeof(BlockHeader);
                nextBlock->isFree = true;
                nextBlock->next = current->next;
                nextBlock->prev = current;
                
                if (current->next) {
                    current->next->prev = nextBlock;
                }

                current->size = size;
                current->next = nextBlock;
            }

            current->isFree = false;
            
#ifdef _WIN32
            LeaveCriticalSection(&cs);
#endif
            return reinterpret_cast<void*>(current + 1);
        }
        current = current->next;
    }

#ifdef _WIN32
    LeaveCriticalSection(&cs);
#endif
    return nullptr; // Out of memory
}

void MemoryAllocator::free(void* ptr) {
    if (!ptr) return;

#ifdef _WIN32
    EnterCriticalSection(&cs);
#else
    std::lock_guard<std::mutex> lock(mtx);
#endif

    BlockHeader* header = static_cast<BlockHeader*>(ptr) - 1;
    header->isFree = true;

    // Coalesce with next
    if (header->next && header->next->isFree) {
        header->size += header->next->size + sizeof(BlockHeader);
        header->next = header->next->next;
        if (header->next) {
            header->next->prev = header;
        }
    }

    // Coalesce with prev
    if (header->prev && header->prev->isFree) {
        header->prev->size += header->size + sizeof(BlockHeader);
        header->prev->next = header->next;
        if (header->next) {
            header->next->prev = header->prev;
        }
    }

#ifdef _WIN32
    LeaveCriticalSection(&cs);
#endif
}

// No longer needed to traverse the whole list
void MemoryAllocator::coalesce() {
    // Logic moved into free() for efficiency
}
