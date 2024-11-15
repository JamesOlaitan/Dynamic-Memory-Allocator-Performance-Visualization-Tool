// custom_allocator
#include "custom_allocator.h"
#include <cmath>
#include <cstring>

CustomAllocator::CustomAllocator(size_t min_order, size_t max_order)
    : minOrder(min_order), maxOrder(max_order), allocationTime(0.0), deallocationTime(0.0) {
    totalSize = 1 << maxOrder;
    memoryPool = std::malloc(totalSize);
    totalFreeMemory = totalSize;

    // Initialize free lists
    freeLists.resize(maxOrder + 1);
    std::memset(memoryPool, 0, totalSize);

    // Add the entire memory pool to the largest free list
    Block* initialBlock = reinterpret_cast<Block*>(memoryPool);
    initialBlock->order = maxOrder;
    initialBlock->free = true;
    freeLists[maxOrder].push_back(initialBlock);
}

CustomAllocator::~CustomAllocator() {
    std::free(memoryPool);
}



/**
 * @brief Allocates memory of at least the given size.
 * @param size The minimum size to allocate.
 * @return Pointer to the allocated memory or nullptr if allocation fails.
 */
void* CustomAllocator::allocate(size_t size) {
    std::lock_guard<std::mutex> lock(allocatorMutex);
    auto startTime = std::chrono::high_resolution_clock::now();

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
            }

            block->free = false;
            totalFreeMemory -= (1 << block->order);

            auto endTime = std::chrono::high_resolution_clock::now();
            recordAllocationTime(std::chrono::duration<double>(endTime - startTime).count());

            // Return the memory address after the block metadata
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
    if (!ptr) return;

    std::lock_guard<std::mutex> lock(allocatorMutex);
    auto startTime = std::chrono::high_resolution_clock::now();

    Block* block = reinterpret_cast<Block*>(reinterpret_cast<char*>(ptr) - sizeof(Block));
    block->free = true;
    totalFreeMemory += (1 << block->order);

    // Merge with buddy blocks if possible
    Block* mergedBlock = mergeBlock(block);
    freeLists[mergedBlock->order].push_back(mergedBlock);

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
    size_t currentOrder = block->order;
    while (currentOrder > targetOrder) {
        currentOrder--;
        size_t size = 1 << currentOrder;
        Block* buddy = reinterpret_cast<Block*>(reinterpret_cast<char*>(block) + size);
        buddy->order = currentOrder;
        buddy->free = true;
        freeLists[currentOrder].push_back(buddy);
        block->order = currentOrder;
    }
    return block;
}



CustomAllocator::Block* CustomAllocator::mergeBlock(CustomAllocator::Block* block) {
    size_t currentOrder = block->order;
    while (currentOrder < maxOrder) {
        Block* buddy = getBuddy(block);
        auto& buddyList = freeLists[currentOrder];

        auto it = std::find(buddyList.begin(), buddyList.end(), buddy);
        if (it != buddyList.end() && buddy->free) {
            // Remove buddy from free list
            buddyList.erase(it);
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
    size_t size = 1 << block->order;
    uintptr_t offset = reinterpret_cast<char*>(block) - reinterpret_cast<char*>(memoryPool);
    uintptr_t buddyOffset = offset ^ size;
    return reinterpret_cast<Block*>(reinterpret_cast<char*>(memoryPool) + buddyOffset);
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