#ifndef CUSTOM_ALLOCATOR_H
#define CUSTOM_ALLOCATOR_H

#include <cstddef>
#include <chrono>
#include <vector>
#include <list>
#include <mutex>

/**
 * @class CustomAllocator
 * @brief A custom memory allocator implementing the buddy allocation algorithm.
 */

class CustomAllocator {
public:
    CustomAllocator(size_t min_order, size_t max_order);
    ~CustomAllocator();

    void* allocate(size_t size);
    void deallocate(void* ptr);

    // Performance metrics
    double getAllocationTime() const;
    double getDeallocationTime() const;
    double getFragmentation() const;

private:
    struct Block {
        size_t order;
        bool free;
        Block* next;
    };

    size_t minOrder;
    size_t maxOrder;
    size_t totalSize;
    void* memoryPool;

    // Frees lists for each order
    std::vector<std::list<Block*>> freeLists;

    // Timing metrics
    double allocationTime;
    double deallocationTime;

    // Fragmentation metrics
    size_t totalFreeMemory;

    // Mutex for thread safety
    std::mutex allocatorMutex;

    // Helper functions
    size_t sizeToOrder(size_t size) const;
    Block* splitBlock(Block* block, size_t targetOrder);
    Block* mergeBlock(Block* block);
    Block* getBuddy(Block* block);

    // Instrumentation functions
    void recordAllocationTime(double time);
    void recordDeallocationTime(double time);
};

#endif // CUSTOM_ALLOCATOR_H