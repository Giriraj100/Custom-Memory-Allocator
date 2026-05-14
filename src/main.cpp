#include <iostream>
#include <vector>
#include <chrono>
#include "MemoryAllocator.hpp"

void benchmarkCustomAllocator(size_t iterations, size_t blockSize) {
    MemoryAllocator allocator(iterations * (blockSize + sizeof(BlockHeader)) * 2);
    auto start = std::chrono::high_resolution_clock::now();

    std::vector<void*> ptrs;
    ptrs.reserve(iterations);

    for (size_t i = 0; i < iterations; ++i) {
        ptrs.push_back(allocator.alloc(blockSize));
    }

    for (void* ptr : ptrs) {
        allocator.free(ptr);
    }

    auto end = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> diff = end - start;
    std::cout << "Custom Allocator time: " << diff.count() << "s\n";
}

void benchmarkStandardAllocator(size_t iterations, size_t blockSize) {
    auto start = std::chrono::high_resolution_clock::now();

    std::vector<void*> ptrs;
    ptrs.reserve(iterations);

    for (size_t i = 0; i < iterations; ++i) {
        ptrs.push_back(std::malloc(blockSize));
    }

    for (void* ptr : ptrs) {
        std::free(ptr);
    }

    auto end = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> diff = end - start;
    std::cout << "Standard malloc/free time: " << diff.count() << "s\n";
}

int main() {
    std::cout << "Testing Custom Memory Allocator...\n";

    MemoryAllocator allocator(1024 * 1024); // 1MB pool

    void* p1 = allocator.alloc(100);
    void* p2 = allocator.alloc(200);
    void* p3 = allocator.alloc(300);

    if (p1 && p2 && p3) {
        std::cout << "Successfully allocated p1, p2, p3\n";
    } else {
        std::cerr << "Failed to allocate memory\n";
        return 1;
    }

    allocator.free(p2);
    void* p4 = allocator.alloc(150); // Should reuse p2's space or split it
    if (p4) {
        std::cout << "Successfully allocated p4 (reused/split space)\n";
    }

    allocator.free(p1);
    allocator.free(p3);
    allocator.free(p4);

    std::cout << "\nStarting Benchmarks...\n";
    const size_t iterations = 10000;
    const size_t blockSize = 32;

    benchmarkCustomAllocator(iterations, blockSize);
    benchmarkStandardAllocator(iterations, blockSize);

    return 0;
}
