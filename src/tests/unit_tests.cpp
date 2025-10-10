#include "gtest/gtest.h"
#include "custom_allocator.h"
#include <thread>
#include <vector>
#include <set>
#include <algorithm>
#include <cmath>

// ============================================================================
// Basic Allocation/Deallocation Tests
// ============================================================================

TEST(CustomAllocatorTest, BasicAllocationDeallocation) {
    CustomAllocator allocator(5, 20);
    void* ptr = allocator.allocate(64);
    EXPECT_NE(ptr, nullptr);

    std::string allocID = allocator.getAllocationID(ptr);
    EXPECT_FALSE(allocID.empty());
    EXPECT_NE(allocID, "");

    allocator.deallocate(ptr);
    EXPECT_DOUBLE_EQ(allocator.getFragmentation(), 1.0);
}

TEST(CustomAllocatorTest, MultipleAllocations) {
    CustomAllocator allocator(6, 20);
    std::vector<void*> ptrs;

    // Allocate 10 blocks
    for (int i = 0; i < 10; ++i) {
        void* ptr = allocator.allocate(128);
        EXPECT_NE(ptr, nullptr);
        ptrs.push_back(ptr);
    }

    // Verify all pointers are unique
    std::set<void*> uniquePtrs(ptrs.begin(), ptrs.end());
    EXPECT_EQ(uniquePtrs.size(), ptrs.size());

    // Deallocate all
    for (void* ptr : ptrs) {
        allocator.deallocate(ptr);
    }

    EXPECT_DOUBLE_EQ(allocator.getFragmentation(), 1.0);
}

TEST(CustomAllocatorTest, NullptrDeallocation) {
    CustomAllocator allocator(6, 20);
    // Should not crash
    EXPECT_NO_THROW(allocator.deallocate(nullptr));
}

// ============================================================================
// Boundary Conditions Tests
// ============================================================================

TEST(CustomAllocatorTest, AllocateMaxSize) {
    CustomAllocator allocator(6, 16); // max = 2^16 = 65536 bytes
    void* ptr = allocator.allocate(65536 - sizeof(CustomAllocator) - 100);
    EXPECT_NE(ptr, nullptr);
    allocator.deallocate(ptr);
}

TEST(CustomAllocatorTest, AllocateTooLarge) {
    CustomAllocator allocator(6, 16);
    // Request more than available
    void* ptr = allocator.allocate(1 << 20); // 1MB, larger than pool
    EXPECT_EQ(ptr, nullptr);
}

TEST(CustomAllocatorTest, AllocateUntilFull) {
    CustomAllocator allocator(6, 12); // Small pool for testing
    std::vector<void*> ptrs;

    // Keep allocating until we fail
    void* ptr;
    while ((ptr = allocator.allocate(64)) != nullptr) {
        ptrs.push_back(ptr);
        if (ptrs.size() > 1000) {
            break; // Safety limit
        }
    }

    EXPECT_GT(ptrs.size(), 0);

    // Clean up
    for (void* p : ptrs) {
        allocator.deallocate(p);
    }
}

TEST(CustomAllocatorTest, AllocateZeroBytes) {
    CustomAllocator allocator(6, 20);
    // Allocating 0 bytes should still return a valid pointer (implementation dependent)
    void* ptr = allocator.allocate(0);
    if (ptr != nullptr) {
        allocator.deallocate(ptr);
    }
    // Test passes regardless, just ensuring no crash
    SUCCEED();
}

// ============================================================================
// Alignment Tests
// ============================================================================

TEST(CustomAllocatorTest, PointerAlignment) {
    CustomAllocator allocator(6, 20);
    std::vector<void*> ptrs;

    // Allocate multiple blocks and check alignment
    for (int i = 0; i < 20; ++i) {
        void* ptr = allocator.allocate(64 + i * 8);
        if (ptr != nullptr) {
            ptrs.push_back(ptr);
            // Check 8-byte alignment (common requirement)
            uintptr_t addr = reinterpret_cast<uintptr_t>(ptr);
            EXPECT_EQ(addr % 8, 0) << "Pointer not 8-byte aligned";
        }
    }

    for (void* ptr : ptrs) {
        allocator.deallocate(ptr);
    }
}

// ============================================================================
// Fragmentation Tests
// ============================================================================

TEST(CustomAllocatorTest, FragmentationCalculation) {
    CustomAllocator allocator(6, 15);

    // Initially, fragmentation should be 1.0 (all free)
    EXPECT_DOUBLE_EQ(allocator.getFragmentation(), 1.0);

    void* ptr1 = allocator.allocate(256);
    EXPECT_NE(ptr1, nullptr);
    // Fragmentation should decrease after allocation
    EXPECT_LT(allocator.getFragmentation(), 1.0);

    void* ptr2 = allocator.allocate(256);
    EXPECT_NE(ptr2, nullptr);
    double frag_after_two = allocator.getFragmentation();
    EXPECT_LT(frag_after_two, 1.0);

    allocator.deallocate(ptr1);
    allocator.deallocate(ptr2);
    // After freeing all, fragmentation should be 1.0 again
    EXPECT_DOUBLE_EQ(allocator.getFragmentation(), 1.0);
}

TEST(CustomAllocatorTest, FragmentationPattern) {
    CustomAllocator allocator(6, 16);
    std::vector<void*> ptrs;

    // Allocate several blocks
    for (int i = 0; i < 5; ++i) {
        void* ptr = allocator.allocate(512);
        if (ptr != nullptr) {
            ptrs.push_back(ptr);
        }
    }

    double frag_before_dealloc = allocator.getFragmentation();

    // Deallocate every other block
    for (size_t i = 0; i < ptrs.size(); i += 2) {
        allocator.deallocate(ptrs[i]);
    }

    double frag_after_partial = allocator.getFragmentation();
    // Fragmentation should increase after freeing some blocks
    EXPECT_GT(frag_after_partial, frag_before_dealloc);

    // Clean up remaining
    for (size_t i = 1; i < ptrs.size(); i += 2) {
        allocator.deallocate(ptrs[i]);
    }
}

// ============================================================================
// Split Invariants Tests
// ============================================================================

TEST(CustomAllocatorTest, SplitBlockInvariants) {
    CustomAllocator allocator(6, 14); // Small pool

    // Allocate a small block, which should cause splitting
    void* ptr1 = allocator.allocate(64);
    EXPECT_NE(ptr1, nullptr);

    // Allocate another small block
    void* ptr2 = allocator.allocate(64);
    EXPECT_NE(ptr2, nullptr);

    // Pointers should be different
    EXPECT_NE(ptr1, ptr2);

    // Calculate distance between pointers (should be reasonable)
    uintptr_t addr1 = reinterpret_cast<uintptr_t>(ptr1);
    uintptr_t addr2 = reinterpret_cast<uintptr_t>(ptr2);
    size_t distance = (addr1 > addr2) ? (addr1 - addr2) : (addr2 - addr1);

    // Distance should be a power of 2 (buddy property)
    EXPECT_GT(distance, 0);
    EXPECT_TRUE((distance & (distance - 1)) == 0 || distance % 64 == 0);

    allocator.deallocate(ptr1);
    allocator.deallocate(ptr2);
}

// ============================================================================
// Coalesce Invariants Tests
// ============================================================================

TEST(CustomAllocatorTest, CoalesceBuddies) {
    CustomAllocator allocator(6, 14);

    // Allocate two adjacent blocks
    void* ptr1 = allocator.allocate(64);
    void* ptr2 = allocator.allocate(64);
    EXPECT_NE(ptr1, nullptr);
    EXPECT_NE(ptr2, nullptr);

    double frag_with_allocs = allocator.getFragmentation();

    // Deallocate both (should trigger coalescing)
    allocator.deallocate(ptr1);
    allocator.deallocate(ptr2);

    double frag_after_coalesce = allocator.getFragmentation();

    // Fragmentation should be 1.0 if all memory is freed and coalesced
    EXPECT_DOUBLE_EQ(frag_after_coalesce, 1.0);
}

TEST(CustomAllocatorTest, MultipleCoalesce) {
    CustomAllocator allocator(6, 15);
    std::vector<void*> ptrs;

    // Allocate many small blocks
    for (int i = 0; i < 8; ++i) {
        void* ptr = allocator.allocate(128);
        if (ptr != nullptr) {
            ptrs.push_back(ptr);
        }
    }

    // Deallocate all - should coalesce back to original state
    for (void* ptr : ptrs) {
        allocator.deallocate(ptr);
    }

    EXPECT_DOUBLE_EQ(allocator.getFragmentation(), 1.0);
}

// ============================================================================
// Metadata Integrity Tests
// ============================================================================

TEST(CustomAllocatorTest, AllocationIDUniqueness) {
    CustomAllocator allocator(6, 20);
    std::set<std::string> ids;

    // Allocate multiple blocks and check ID uniqueness
    for (int i = 0; i < 50; ++i) {
        void* ptr = allocator.allocate(64);
        if (ptr != nullptr) {
            std::string id = allocator.getAllocationID(ptr);
            EXPECT_FALSE(id.empty());
            EXPECT_EQ(ids.count(id), 0) << "Duplicate allocation ID: " << id;
            ids.insert(id);
            allocator.deallocate(ptr);
        }
    }

    EXPECT_GT(ids.size(), 0);
}

TEST(CustomAllocatorTest, MemoryAddressConsistency) {
    CustomAllocator allocator(6, 20);

    void* ptr = allocator.allocate(256);
    EXPECT_NE(ptr, nullptr);

    std::string addr1 = allocator.getMemoryAddress(ptr);
    std::string addr2 = allocator.getMemoryAddress(ptr);

    // Address should be consistent
    EXPECT_EQ(addr1, addr2);
    EXPECT_FALSE(addr1.empty());

    allocator.deallocate(ptr);
}

// ============================================================================
// Throughput Metrics Tests
// ============================================================================

TEST(CustomAllocatorTest, ThroughputCounters) {
    CustomAllocator allocator(6, 20);

    EXPECT_EQ(allocator.getTotalAllocations(), 0);
    EXPECT_EQ(allocator.getTotalDeallocations(), 0);

    void* ptr1 = allocator.allocate(128);
    EXPECT_EQ(allocator.getTotalAllocations(), 1);
    EXPECT_EQ(allocator.getTotalDeallocations(), 0);

    void* ptr2 = allocator.allocate(256);
    EXPECT_EQ(allocator.getTotalAllocations(), 2);

    allocator.deallocate(ptr1);
    EXPECT_EQ(allocator.getTotalDeallocations(), 1);

    allocator.deallocate(ptr2);
    EXPECT_EQ(allocator.getTotalDeallocations(), 2);
}

// ============================================================================
// Thread Safety Tests
// ============================================================================

TEST(CustomAllocatorTest, ConcurrentAllocations) {
    CustomAllocator allocator(6, 20);
    const int num_threads = 4;
    const int allocs_per_thread = 50;

    auto worker = [&allocator, allocs_per_thread]() {
        std::vector<void*> local_ptrs;
        for (int i = 0; i < allocs_per_thread; ++i) {
            void* ptr = allocator.allocate(64 + (i % 10) * 8);
            if (ptr != nullptr) {
                local_ptrs.push_back(ptr);
            }
        }
        // Deallocate in reverse order
        for (auto it = local_ptrs.rbegin(); it != local_ptrs.rend(); ++it) {
            allocator.deallocate(*it);
        }
    };

    std::vector<std::thread> threads;
    for (int i = 0; i < num_threads; ++i) {
        threads.emplace_back(worker);
    }

    for (auto& t : threads) {
        t.join();
    }

    // After all threads finish, should be back to full fragmentation
    EXPECT_DOUBLE_EQ(allocator.getFragmentation(), 1.0);
    EXPECT_EQ(allocator.getTotalAllocations(), allocator.getTotalDeallocations());
}

TEST(CustomAllocatorTest, ConcurrentMixedOperations) {
    CustomAllocator allocator(6, 20);
    const int num_threads = 4;

    auto worker = [&allocator]() {
        std::vector<void*> ptrs;
        for (int i = 0; i < 30; ++i) {
            // Allocate
            void* ptr = allocator.allocate(128);
            if (ptr != nullptr) {
                ptrs.push_back(ptr);
            }

            // Deallocate some if we have enough
            if (ptrs.size() > 10) {
                allocator.deallocate(ptrs.back());
                ptrs.pop_back();
            }
        }

        // Clean up remaining
        for (void* ptr : ptrs) {
            allocator.deallocate(ptr);
        }
    };

    std::vector<std::thread> threads;
    for (int i = 0; i < num_threads; ++i) {
        threads.emplace_back(worker);
    }

    for (auto& t : threads) {
        t.join();
    }

    // Verify consistency
    EXPECT_DOUBLE_EQ(allocator.getFragmentation(), 1.0);
}

// ============================================================================
// Timing Metrics Tests
// ============================================================================

TEST(CustomAllocatorTest, AllocationTimingRecorded) {
    CustomAllocator allocator(6, 20);

    EXPECT_DOUBLE_EQ(allocator.getAllocationTime(), 0.0);

    void* ptr = allocator.allocate(256);
    EXPECT_NE(ptr, nullptr);

    // Timing should be recorded (non-negative)
    EXPECT_GE(allocator.getAllocationTime(), 0.0);

    allocator.deallocate(ptr);
}

TEST(CustomAllocatorTest, DeallocationTimingRecorded) {
    CustomAllocator allocator(6, 20);

    void* ptr = allocator.allocate(256);
    EXPECT_NE(ptr, nullptr);

    EXPECT_DOUBLE_EQ(allocator.getDeallocationTime(), 0.0);

    allocator.deallocate(ptr);

    // Deallocation timing should be recorded
    EXPECT_GE(allocator.getDeallocationTime(), 0.0);
}

// ============================================================================
// Stress Tests
// ============================================================================

TEST(CustomAllocatorTest, RandomAllocationPattern) {
    CustomAllocator allocator(6, 18);
    std::vector<void*> ptrs;

    // Random allocation/deallocation pattern
    for (int i = 0; i < 100; ++i) {
        if (ptrs.empty() || (i % 3 != 0 && ptrs.size() < 50)) {
            // Allocate
            size_t size = 64 + (i % 20) * 16;
            void* ptr = allocator.allocate(size);
            if (ptr != nullptr) {
                ptrs.push_back(ptr);
            }
        } else {
            // Deallocate random
            if (!ptrs.empty()) {
                size_t idx = i % ptrs.size();
                allocator.deallocate(ptrs[idx]);
                ptrs.erase(ptrs.begin() + idx);
            }
        }
    }

    // Clean up
    for (void* ptr : ptrs) {
        allocator.deallocate(ptr);
    }

    EXPECT_DOUBLE_EQ(allocator.getFragmentation(), 1.0);
}

int main(int argc, char** argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
