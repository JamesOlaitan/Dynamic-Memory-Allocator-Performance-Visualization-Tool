#ifndef CUSTOM_ALLOCATOR_H
#define CUSTOM_ALLOCATOR_H

#include <atomic>
#include <chrono>
#include <cstddef>
#include <limits>
#include <list>
#include <mutex>
#include <sstream>
#include <string>
#include <vector>

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

    // Public methods to access allocation information
    std::string getAllocationID(void* ptr);
    std::string getMemoryAddress(void* ptr);

    // Getter methods for throughput metrics
    size_t getTotalAllocations() const;
    size_t getTotalDeallocations() const;

   private:
    struct alignas(std::max_align_t) Block {
        size_t order;
        bool free;
        Block* next;
        size_t allocationIndex;
    };

    static constexpr size_t INVALID_ALLOCATION_ID = std::numeric_limits<size_t>::max();

    size_t minOrder;
    size_t maxOrder;
    size_t totalSize;
    void* memoryPool;

    // Free lists for each order
    std::vector<std::list<Block*>> freeLists;

    // Timing metrics
    double allocationTime;
    double deallocationTime;

    // Fragmentation metrics
    size_t totalFreeMemory;

    // Mutex for thread safety
    std::mutex allocatorMutex;

    // Allocation counters for generating unique IDs and tracking throughput
    std::atomic<size_t> allocationCounter;
    std::atomic<size_t> totalAllocations;
    std::atomic<size_t> totalDeallocations;

    // Helper functions
    size_t sizeToOrder(size_t size) const;
    Block* splitBlock(Block* block, size_t targetOrder);
    Block* mergeBlock(Block* block);
    Block* getBuddy(Block* block);
    bool isValidBlock(Block* block) const;

    // Instrumentation functions
    void recordAllocationTime(double time);
    void recordDeallocationTime(double time);

    // Generates a unique allocation index for ID formatting
    size_t generateAllocationIndex();
};

#endif  // CUSTOM_ALLOCATOR_H
