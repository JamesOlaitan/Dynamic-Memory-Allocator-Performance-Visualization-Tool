#include <iostream>

#include "custom_allocator.h"

// Main function example usage
int main() {
    // Initializes allocator with min order 5 (32 bytes) and max order 20 (~1 MB)
    CustomAllocator allocator(5, 20);

    // Example allocations
    void* ptr1 = allocator.allocate(64);   // Allocates 64 bytes
    void* ptr2 = allocator.allocate(128);  // Allocates 128 bytes

    // Deallocates memory
    allocator.deallocate(ptr1);
    allocator.deallocate(ptr2);

    // Gets performance metrics
    double allocTime = allocator.getAllocationTime();
    double deallocTime = allocator.getDeallocationTime();
    double fragmentation = allocator.getFragmentation();

    // Outputs metrics
    std::cout << "Allocation Time: " << allocTime << " seconds" << std::endl;
    std::cout << "Deallocation Time: " << deallocTime << " seconds" << std::endl;
    std::cout << "Fragmentation: " << fragmentation * 100 << "%" << std::endl;

    return 0;
}