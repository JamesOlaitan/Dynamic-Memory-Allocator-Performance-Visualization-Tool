#include "gtest/gtest.h"
#include "custom_allocator.h"

// Google Test code
TEST(CustomAllocatorTest, AllocationDeallocation) {
    CustomAllocator allocator(5, 20);
    void* ptr = allocator.allocate(64);
    EXPECT_NE(ptr, nullptr);
    allocator.deallocate(ptr);
    EXPECT_DOUBLE_EQ(allocator.getFragmentation(), 1.0);
}

int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
