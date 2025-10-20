// custom_allocator.cpp
#include "custom_allocator.h"

#include <algorithm>
#include <cmath>
#include <sstream>
#include <thread>

CustomAllocator::CustomAllocator(size_t min_order, size_t max_order)
    : minOrder(min_order),
      maxOrder(max_order),
      allocationTime(0.0),
      deallocationTime(0.0),
      allocationCounter(0),
      totalAllocations(0),
      totalDeallocations(0) {  // Initializes atomic counters
    totalSize = static_cast<size_t>(1) << maxOrder;
    memoryPool = std::malloc(totalSize);
    if (!memoryPool) {
        throw std::bad_alloc();
    }
    totalFreeMemory = totalSize;

    // Initialize free lists
    freeLists.resize(maxOrder + 1);

    // Add the entire memory pool to the largest free list
    Block* initialBlock = reinterpret_cast<Block*>(memoryPool);
    initialBlock->order = maxOrder;
    initialBlock->free = true;
    initialBlock->next = nullptr;
    initialBlock->allocationIndex = INVALID_ALLOCATION_ID;
    freeLists[maxOrder].push_back(initialBlock);
}

CustomAllocator::~CustomAllocator() {
    std::free(memoryPool);
}

/**
 * @brief Generates a unique Allocation ID.
 * @return A unique allocation ID as a string.
 */
size_t CustomAllocator::generateAllocationIndex() {
    return allocationCounter.fetch_add(1, std::memory_order_relaxed);
}

std::string CustomAllocator::getAllocationID(void* ptr) {
    std::lock_guard<std::mutex> lock(allocatorMutex);
    if (!ptr)
        return "";

    // Validate that the pointer is within our memory pool
    char* ptrChar = reinterpret_cast<char*>(ptr);
    char* poolStart = reinterpret_cast<char*>(memoryPool);
    char* poolEnd = poolStart + totalSize;

    if (ptrChar < poolStart || ptrChar >= poolEnd) {
        return "";  // Invalid pointer
    }

    Block* block = reinterpret_cast<Block*>(ptrChar - sizeof(Block));

    // Validate that the block is within bounds
    if (reinterpret_cast<char*>(block) < poolStart || reinterpret_cast<char*>(block) >= poolEnd) {
        return "";  // Invalid block
    }

    if (block->allocationIndex == INVALID_ALLOCATION_ID) {
        return "";
    }

    return "Alloc" + std::to_string(block->allocationIndex);
}

std::string CustomAllocator::getMemoryAddress(void* ptr) {
    std::ostringstream memAddrStream;
    memAddrStream << ptr;
    return memAddrStream.str();
}

/**
 * @brief Allocates memory of at least the given size.
 * @param size The minimum size to allocate.
 * @return Pointer to the allocated memory or nullptr if allocation fails.
 */
void* CustomAllocator::allocate(size_t size) {
    std::lock_guard<std::mutex> lock(allocatorMutex);
    auto startTime = std::chrono::high_resolution_clock::now();

    // Handle zero-size allocation
    if (size == 0) {
        size = 1;  // Allocate at least 1 byte
    }

    size_t requiredOrder = sizeToOrder(size + sizeof(Block));
    if (requiredOrder > maxOrder) {
        // Cannot allocate memory larger than pool
        return nullptr;
    }

    for (size_t order = requiredOrder; order <= maxOrder; ++order) {
        if (!freeLists[order].empty()) {
            Block* block = freeLists[order].front();
            freeLists[order].pop_front();

            // Split blocks until we reach the required order
            while (order > requiredOrder) {
                block = splitBlock(block, --order);
                if (!block) {
                    return nullptr;  // Split failed
                }
            }

            block->free = false;
            block->next = nullptr;  // Initialize next pointer
            block->allocationIndex = generateAllocationIndex();
            totalFreeMemory -= (1 << block->order);

            totalAllocations.fetch_add(1, std::memory_order_relaxed);

            auto endTime = std::chrono::high_resolution_clock::now();
            recordAllocationTime(std::chrono::duration<double>(endTime - startTime).count());

            // Returns the memory address after the block metadata
            return reinterpret_cast<void*>(reinterpret_cast<char*>(block) + sizeof(Block));
        }
    }

    // No suitable block found
    return nullptr;
}

/**
 * @brief Deallocates the memory pointed to by ptr.
 * @param ptr Pointer to the memory to deallocate.
 */
void CustomAllocator::deallocate(void* ptr) {
    if (!ptr)
        return;

    std::lock_guard<std::mutex> lock(allocatorMutex);
    auto startTime = std::chrono::high_resolution_clock::now();

    // Validate that the pointer is within our memory pool
    char* ptrChar = reinterpret_cast<char*>(ptr);
    char* poolStart = reinterpret_cast<char*>(memoryPool);
    char* poolEnd = poolStart + totalSize;

    if (ptrChar < poolStart || ptrChar >= poolEnd) {
        return;  // Invalid pointer, ignore
    }

    Block* block = reinterpret_cast<Block*>(ptrChar - sizeof(Block));

    block->free = true;
    block->next = nullptr;  // Initialize next pointer
    block->allocationIndex = INVALID_ALLOCATION_ID;
    totalFreeMemory += (1 << block->order);

    totalDeallocations.fetch_add(1, std::memory_order_relaxed);

    // Merge with buddy blocks if possible
    Block* mergedBlock = mergeBlock(block);
    if (mergedBlock) {
        mergedBlock->next = nullptr;  // Initialize next pointer
        mergedBlock->allocationIndex = INVALID_ALLOCATION_ID;
        freeLists[mergedBlock->order].push_back(mergedBlock);
    }

    auto endTime = std::chrono::high_resolution_clock::now();
    recordDeallocationTime(std::chrono::duration<double>(endTime - startTime).count());
}

size_t CustomAllocator::sizeToOrder(size_t size) const {
    size_t order = minOrder;
    size_t blockSize = 1 << order;
    while (blockSize < size && order < maxOrder) {
        ++order;
        blockSize <<= 1;
    }
    return order;
}

CustomAllocator::Block* CustomAllocator::splitBlock(CustomAllocator::Block* block, size_t targetOrder) {
    if (!block) {
        return nullptr;
    }

    size_t currentOrder = block->order;
    while (currentOrder > targetOrder) {
        currentOrder--;
        size_t size = 1 << currentOrder;
        Block* buddy = reinterpret_cast<Block*>(reinterpret_cast<char*>(block) + size);

        // Initialize the new buddy block metadata
        buddy->order = currentOrder;
        buddy->free = true;
        buddy->next = nullptr;
        buddy->allocationIndex = INVALID_ALLOCATION_ID;
        freeLists[currentOrder].push_back(buddy);
        block->order = currentOrder;
        block->allocationIndex = INVALID_ALLOCATION_ID;
    }
    return block;
}

CustomAllocator::Block* CustomAllocator::mergeBlock(CustomAllocator::Block* block) {
    if (!block) {
        return nullptr;
    }

    size_t currentOrder = block->order;
    while (currentOrder < maxOrder) {
        Block* buddy = getBuddy(block);
        if (!buddy) {
            break;
        }

        auto& buddyList = freeLists[currentOrder];

        auto it = std::find(buddyList.begin(), buddyList.end(), buddy);
        if (it != buddyList.end() && buddy->free) {
            // Remove buddy from free list
            buddyList.erase(it);
            buddy->allocationIndex = INVALID_ALLOCATION_ID;
            if (buddy > block) {
                block->order++;
            } else {
                block = buddy;
                block->order++;
            }
            currentOrder++;
        } else {
            break;
        }
    }
    return block;
}

CustomAllocator::Block* CustomAllocator::getBuddy(CustomAllocator::Block* block) {
    if (!block) {
        return nullptr;
    }

    size_t size = 1 << block->order;
    uintptr_t offset = reinterpret_cast<char*>(block) - reinterpret_cast<char*>(memoryPool);
    uintptr_t buddyOffset = offset ^ size;

    // Validate buddy offset is within bounds
    if (buddyOffset >= totalSize) {
        return nullptr;
    }

    Block* buddy = reinterpret_cast<Block*>(reinterpret_cast<char*>(memoryPool) + buddyOffset);
    return buddy;
}

void CustomAllocator::recordAllocationTime(double time) {
    allocationTime += time;
}

void CustomAllocator::recordDeallocationTime(double time) {
    deallocationTime += time;
}

double CustomAllocator::getAllocationTime() const {
    return allocationTime;
}

double CustomAllocator::getDeallocationTime() const {
    return deallocationTime;
}

double CustomAllocator::getFragmentation() const {
    return static_cast<double>(totalFreeMemory) / totalSize;
}

size_t CustomAllocator::getTotalAllocations() const {
    return totalAllocations.load(std::memory_order_relaxed);
}

size_t CustomAllocator::getTotalDeallocations() const {
    return totalDeallocations.load(std::memory_order_relaxed);
}

bool CustomAllocator::isValidBlock(Block* block) const {
    if (!block || !memoryPool) {
        return false;
    }

    char* blockChar = reinterpret_cast<char*>(block);
    char* poolStart = reinterpret_cast<char*>(memoryPool);
    char* poolEnd = poolStart + totalSize;

    // Check if block is within memory pool bounds
    if (blockChar < poolStart || blockChar >= poolEnd) {
        return false;
    }

    // Check if block order is valid
    if (block->order < minOrder || block->order > maxOrder) {
        return false;
    }

    // Check if block size is valid (must be power of 2)
    size_t blockSize = 1 << block->order;
    if (blockSize == 0) {
        return false;
    }

    // Check if block + size doesn't exceed pool bounds
    if (blockChar + blockSize > poolEnd) {
        return false;
    }

    return true;
}
