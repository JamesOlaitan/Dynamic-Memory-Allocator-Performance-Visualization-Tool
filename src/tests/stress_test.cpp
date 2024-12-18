/**
 * @file stress_test.cpp
 * @brief Stress testing framework for the custom memory allocator.
 *
 * This file implements stress tests to evaluate the performance and stability
 * of the custom memory allocator under extreme conditions. It leverages the
 * Google Benchmark library to conduct microbenchmarks and custom benchmarks,
 * ensuring accurate timing and fragmentation data.
 */

#include <benchmark/benchmark.h>
#include <iostream>
#include <vector>
#include <random>
#include <string>
#include <fstream>
#include <memory>
#include <cstdlib>
#include "custom_allocator.h"
#include "data_logger.h"
#include <chrono>
#include <thread>
#include <atomic>

/**
 * @class AllocatorFixture
 * @brief Fixture class for setting up and tearing down the CustomAllocator and DataLogger.
 *
 * This fixture ensures that the CustomAllocator and DataLogger are initialized before each benchmark
 * and properly cleaned up afterward.
 */
class AllocatorFixture : public benchmark::Fixture {
public:
    /**
     * @brief Set up the allocator and DataLogger before each benchmark.
     * 
     * Initializes the CustomAllocator with specified min and max orders.
     *
     * @param state Benchmark state.
     */
    void SetUp(const ::benchmark::State& state) override {
        // Define min_order and max_order
        size_t min_order = 6;  // Example: 2^6 = 64 bytes
        size_t max_order = 16; // Example: 2^16 = 65536 bytes

        // Initialize the CustomAllocator
        allocator = new CustomAllocator(min_order, max_order);

        // Initialize DataLogger with a specific log file
        std::string logFilename = "stress_test_data.csv";
        dataLogger = new DataLogger(logFilename);
    }

    /**
     * @brief Tear down the allocator and DataLogger after each benchmark.
     * 
     * Logs summary data and deletes the CustomAllocator and DataLogger instances.
     *
     * @param state Benchmark state.
     */
    void TearDown(const ::benchmark::State& state) override {
        if (allocator && dataLogger) {
            // Retrieve performance metrics from allocator
            double allocTime = allocator->getAllocationTime();       // Total allocation time in seconds
            double deallocTime = allocator->getDeallocationTime();   // Total deallocation time in seconds
            double fragmentation = allocator->getFragmentation();     // Fragmentation ratio (0.0 to 1.0)

            // Retrieve total allocations and deallocations from allocator
            size_t totalAllocs = allocator->getTotalAllocations();
            size_t totalDeallocs = allocator->getTotalDeallocations();

            // Compute throughput (operations per second)
            double allocThroughput = (allocTime > 0.0) ? (static_cast<double>(totalAllocs) / allocTime) : 0.0;
            double deallocThroughput = (deallocTime > 0.0) ? (static_cast<double>(totalDeallocs) / deallocTime) : 0.0;

            // Log summary
            std::string summary = "Stress Test Summary";
            dataLogger->logSummary(summary, allocThroughput, deallocThroughput, fragmentation * 100.0); // Convert to percentage

            // Clean up
            delete allocator;
            allocator = nullptr;

            delete dataLogger;
            dataLogger = nullptr;
        }
    }

protected:
    CustomAllocator* allocator; /**< Pointer to the CustomAllocator instance */
    DataLogger* dataLogger;     /**< Pointer to the DataLogger instance */

    // Removed the following atomic counters as they are now handled within CustomAllocator
    // std::atomic<size_t> totalAllocations{0};
    // std::atomic<size_t> totalDeallocations{0};
};

/**
 * @brief Benchmark to measure allocation speed.
 *
 * This microbenchmark measures the time taken to perform a large number of
 * allocation and deallocation operations using the CustomAllocator.
 *
 * @param state Benchmark state.
 */
BENCHMARK_DEFINE_F(AllocatorFixture, AllocationSpeed)(benchmark::State& state) {
    const size_t num_allocations = state.range(0);
    std::vector<void*> pointers;
    pointers.reserve(num_allocations);

    for (auto _ : state) {
        // Allocation phase
        for (size_t i = 0; i < num_allocations; ++i) {
            void* ptr = allocator->allocate(128);  // Allocate 128 bytes
            if (ptr) {
                pointers.push_back(ptr);
                // Allocation counter is already handled within CustomAllocator
            }
        }

        // Deallocation phase
        for (void* ptr : pointers) {
            allocator->deallocate(ptr);
            // Deallocation counter is already handled within CustomAllocator
        }
        pointers.clear();

        // Record the number of operations
        state.PauseTiming();
        // Optionally, log or perform additional checks here
        state.ResumeTiming();
    }

    state.SetComplexityN(num_allocations);
}

// Register the benchmark with a range of allocation counts
BENCHMARK_REGISTER_F(AllocatorFixture, AllocationSpeed)
    ->Arg(1000)
    ->Arg(10000)
    ->Arg(100000)
    ->Unit(benchmark::kMicrosecond)
    ->Complexity();

/**
 * @brief Benchmark to measure memory fragmentation.
 *
 * This custom benchmark assesses the allocator's ability to handle
 * fragmentation by performing varied allocation and deallocation patterns.
 *
 * @param state Benchmark state.
 */
BENCHMARK_DEFINE_F(AllocatorFixture, MemoryFragmentation)(benchmark::State& state) {
    const size_t num_operations = state.range(0);
    std::vector<void*> pointers;
    pointers.reserve(num_operations);
    std::mt19937 rng(42);  // Fixed seed for reproducibility
    std::uniform_int_distribution<size_t> size_dist(64, 1024);  // Allocation sizes between 64 and 1024 bytes
    std::uniform_int_distribution<int> op_dist(0, 1);  // 0 for allocate, 1 for deallocate

    for (auto _ : state) {
        for (size_t i = 0; i < num_operations; ++i) {
            int operation = op_dist(rng);
            if (operation == 0) {  // Allocate
                size_t size = size_dist(rng);
                void* ptr = allocator->allocate(size);
                if (ptr) {
                    pointers.push_back(ptr);
                    // Allocation counter is already handled within CustomAllocator
                }
            } else {  // Deallocate
                if (!pointers.empty()) {
                    size_t index = rng() % pointers.size();
                    allocator->deallocate(pointers[index]);
                    // Deallocation counter is already handled within CustomAllocator
                    pointers[index] = pointers.back();
                    pointers.pop_back();
                }
            }
        }

        // Deallocate any remaining pointers to prevent memory leaks
        for (void* ptr : pointers) {
            allocator->deallocate(ptr);
            // Deallocation counter is already handled within CustomAllocator
        }
        pointers.clear();

        // Record the number of operations
        state.PauseTiming();
        // Optionally, log or perform additional checks here
        state.ResumeTiming();
    }

    state.SetComplexityN(num_operations);
}

// Register the benchmark with a range of operation counts
BENCHMARK_REGISTER_F(AllocatorFixture, MemoryFragmentation)
    ->Arg(1000)
    ->Arg(10000)
    ->Arg(100000)
    ->Unit(benchmark::kMicrosecond)
    ->Complexity();

/**
 * @brief Benchmark to determine the allocator's maximum capacity.
 *
 * This stress test increases the number of allocations until the allocator fails,
 * helping to identify the allocator's limits.
 *
 * @param state Benchmark state.
 */
BENCHMARK_DEFINE_F(AllocatorFixture, MaxLoadTest)(benchmark::State& state) {
    size_t max_allocations = 0;

    for (auto _ : state) {
        std::vector<void*> pointers;
        pointers.reserve(1000);  // Reserve space for allocations

        // Allocate until failure
        while (true) {
            void* ptr = allocator->allocate(128);  // Allocate 128 bytes
            if (!ptr) {
                break;  // Allocation failed
            }
            pointers.push_back(ptr);
            max_allocations++;
            // Allocation counter is already handled within CustomAllocator
        }

        // Deallocate all allocated pointers
        for (void* ptr : pointers) {
            allocator->deallocate(ptr);
            // Deallocation counter is already handled within CustomAllocator
        }
        pointers.clear();

        // Record the number of allocations
        state.PauseTiming();
        state.counters["Max Allocations"] = benchmark::Counter(max_allocations, benchmark::Counter::kIsIterationInvariantRate);
        state.ResumeTiming();
    }

    state.SetComplexityN(max_allocations);
}

// Register the benchmark without specific arguments, it runs until failure
BENCHMARK_REGISTER_F(AllocatorFixture, MaxLoadTest)
    ->Unit(benchmark::kMicrosecond)
    ->Complexity();

int main(int argc, char** argv) {
    // Initialize Google Benchmark
    ::benchmark::Initialize(&argc, argv);

    // Run all registered benchmarks
    ::benchmark::RunSpecifiedBenchmarks();

    return 0;
}