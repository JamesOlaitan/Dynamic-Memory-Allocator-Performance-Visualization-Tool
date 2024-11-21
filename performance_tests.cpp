/**
 * @file performance_tests.cpp
 * @brief Performance benchmarking for CustomAllocator.
 *
 * This file contains functions to benchmark the performance of the CustomAllocator
 * under various workloads. It measures allocation/deallocation times, throughput,
 * and memory fragmentation. Results are logged using the DataLogger class.
 */

#include <iostream>
#include <vector>
#include <random>
#include <chrono>
#include <string>
#include <iomanip>
#include <sstream>
#include <thread>
#include "cxxopts.hpp"
#include "custom_allocator.h"
#include "data_logger.h"

/**
 * @brief Performs fixed-size allocation and deallocation benchmark.
 *
 * Allocates and deallocates memory blocks of a fixed size repeatedly, measuring
 * the time taken for each operation and logging the results.
 *
 * @param allocator Reference to the CustomAllocator instance.
 * @param blockSize Size of each memory block to allocate (in bytes).
 * @param numOperations Number of allocation/deallocation operations to perform.
 * @param logger Reference to the DataLogger instance for logging performance metrics.
 */
void fixedSizeBenchmark(CustomAllocator& allocator, size_t blockSize, size_t numOperations, DataLogger& logger);

/**
 * @brief Performs variable-size allocation and deallocation benchmark.
 *
 * Allocates and deallocates memory blocks with sizes following a uniform distribution,
 * measuring the time taken for each operation and logging the results.
 *
 * @param allocator Reference to the CustomAllocator instance.
 * @param minBlockSize Minimum size of memory blocks to allocate (in bytes).
 * @param maxBlockSize Maximum size of memory blocks to allocate (in bytes).
 * @param numOperations Number of allocation/deallocation operations to perform.
 * @param logger Reference to the DataLogger instance for logging performance metrics.
 */
void variableSizeBenchmark(CustomAllocator& allocator, size_t minBlockSize, size_t maxBlockSize, size_t numOperations, DataLogger& logger);

/**
 * @brief Measures the throughput of allocation and deallocation operations.
 *
 * Performs a high-frequency allocation and deallocation of memory blocks to measure
 * the allocator's throughput in operations per second.
 *
 * @param allocator Reference to the CustomAllocator instance.
 * @param blockSize Size of each memory block to allocate (in bytes).
 * @param duration Duration of the benchmark in seconds.
 * @param logger Reference to the DataLogger instance for logging performance metrics.
 */
void throughputBenchmark(CustomAllocator& allocator, size_t blockSize, double duration, DataLogger& logger);

/**
 * @brief Entry point for the performance tests.
 *
 * Parses command-line arguments to select the type of benchmark and its parameters,
 * then executes the corresponding benchmark function.
 *
 * @param argc Number of command-line arguments.
 * @param argv Array of command-line argument strings.
 * @return Exit code.
 */
int main(int argc, char* argv[]) {
    std::cout << "Running performance_tests.cpp main function." << std::endl;

    // Define and parse command-line options
    cxxopts::Options options("performance_tests", "CustomAllocator Performance Benchmarking");

    options.add_options()
        ("benchmark", "Benchmark type [fixed|variable|throughput]", cxxopts::value<std::string>()->default_value("fixed"))
        ("block-size", "Block size in bytes (for fixed and throughput benchmarks)", cxxopts::value<size_t>()->default_value("64"))
        ("min-block-size", "Minimum block size in bytes (for variable benchmark)", cxxopts::value<size_t>()->default_value("32"))
        ("max-block-size", "Maximum block size in bytes (for variable benchmark)", cxxopts::value<size_t>()->default_value("512"))
        ("num-ops", "Number of operations (for fixed and variable benchmarks)", cxxopts::value<size_t>()->default_value("100000"))
        ("duration", "Duration in seconds (for throughput benchmark)", cxxopts::value<double>()->default_value("10.0"))
        ("output-file", "Path to output CSV file", cxxopts::value<std::string>()->default_value("performance_data.csv"))
        ("help", "Print help");

    auto result = options.parse(argc, argv);

    if (result.count("help")) {
        std::cout << options.help() << std::endl;
        return 0;
    }

    // Extract options
    std::string benchmarkType = result["benchmark"].as<std::string>();
    size_t blockSize = result["block-size"].as<size_t>();
    size_t minBlockSize = result["min-block-size"].as<size_t>();
    size_t maxBlockSize = result["max-block-size"].as<size_t>();
    size_t numOperations = result["num-ops"].as<size_t>();
    double duration = result["duration"].as<double>();
    std::string outputFile = result["output-file"].as<std::string>();

    // Initialize the DataLogger
    DataLogger logger(outputFile);

    // Initialize the allocator with default or specified parameters
    size_t minOrder = 5;  // Example values; adjust as needed
    size_t maxOrder = 20;
    CustomAllocator allocator(minOrder, maxOrder);

    // Execute the selected benchmark
    if (benchmarkType == "fixed") {
        std::cout << "Starting Fixed-Size Allocation Benchmark..." << std::endl;
        fixedSizeBenchmark(allocator, blockSize, numOperations, logger);
    }
    else if (benchmarkType == "variable") {
        std::cout << "Starting Variable-Size Allocation Benchmark..." << std::endl;
        variableSizeBenchmark(allocator, minBlockSize, maxBlockSize, numOperations, logger);
    }
    else if (benchmarkType == "throughput") {
        std::cout << "Starting Throughput Benchmark..." << std::endl;
        throughputBenchmark(allocator, blockSize, duration, logger);
    }
    else {
        std::cerr << "Invalid benchmark type specified. Use [fixed|variable|throughput]." << std::endl;
        return 1;
    }

    std::cout << "Performance Benchmarking Completed." << std::endl;
    return 0;
}

void fixedSizeBenchmark(CustomAllocator& allocator, size_t blockSize, size_t numOperations, DataLogger& logger) {
    std::vector<void*> pointers;
    pointers.reserve(numOperations);

    std::vector<std::string> allocationIDs;
    allocationIDs.reserve(numOperations);

    for (size_t i = 0; i < numOperations; ++i) {
        // Time the allocation
        auto allocStart = std::chrono::high_resolution_clock::now();
        void* ptr = allocator.allocate(blockSize);
        auto allocEnd = std::chrono::high_resolution_clock::now();
        double allocTime = std::chrono::duration<double, std::micro>(allocEnd - allocStart).count(); // in microseconds

        if (ptr == nullptr) {
            std::cerr << "Allocation failed at iteration " << i << std::endl;
            break;
        }

        // Get allocation ID
        std::string allocationID = allocator.getAllocationID(ptr);

        pointers.push_back(ptr);
        allocationIDs.push_back(allocationID);

        // Generate timestamp
        auto now = std::chrono::system_clock::now();
        std::time_t now_time_t = std::chrono::system_clock::to_time_t(now);
        std::tm now_tm = *std::localtime(&now_time_t);
        std::ostringstream timestampStream;
        timestampStream << std::put_time(&now_tm, "%Y-%m-%d %H:%M:%S");
        std::string timestamp = timestampStream.str();

        // Get thread ID
        std::ostringstream threadIDStream;
        threadIDStream << std::this_thread::get_id();
        std::string threadID = threadIDStream.str();

        // Get memory address
        std::string memoryAddress = allocator.getMemoryAddress(ptr);

        // Source and CallStack (placeholders)
        std::string source = __FUNCTION__; // Function name
        std::string callStack = "fixedSizeBenchmark";

        // Log allocation time and fragmentation
        logger.log(timestamp, "Allocation", blockSize, allocTime, allocator.getFragmentation(),
                   source, callStack, memoryAddress, threadID, allocationID);
    }

    // Deallocate all pointers
    for (size_t i = 0; i < pointers.size(); ++i) {
        // Time the deallocation
        auto deallocStart = std::chrono::high_resolution_clock::now();
        allocator.deallocate(pointers[i]);
        auto deallocEnd = std::chrono::high_resolution_clock::now();
        double deallocTime = std::chrono::duration<double, std::micro>(deallocEnd - deallocStart).count(); // in microseconds

        // Generate timestamp
        auto now = std::chrono::system_clock::now();
        std::time_t now_time_t = std::chrono::system_clock::to_time_t(now);
        std::tm now_tm = *std::localtime(&now_time_t);
        std::ostringstream timestampStream;
        timestampStream << std::put_time(&now_tm, "%Y-%m-%d %H:%M:%S");
        std::string timestamp = timestampStream.str();

        // Get thread ID
        std::ostringstream threadIDStream;
        threadIDStream << std::this_thread::get_id();
        std::string threadID = threadIDStream.str();

        // Get memory address
        std::string memoryAddress = allocator.getMemoryAddress(pointers[i]);

        // Source and CallStack (placeholders)
        std::string source = __FUNCTION__; // Function name
        std::string callStack = "fixedSizeBenchmark";

        // Log deallocation time and fragmentation
        logger.log(timestamp, "Deallocation", blockSize, deallocTime, allocator.getFragmentation(),
                   source, callStack, memoryAddress, threadID, allocationIDs[i]);
    }

    std::cout << "Fixed-Size Allocation Benchmark completed with " << numOperations << " operations." << std::endl;
}

void variableSizeBenchmark(CustomAllocator& allocator, size_t minBlockSize, size_t maxBlockSize, size_t numOperations, DataLogger& logger) {
    std::vector<void*> pointers;
    pointers.reserve(numOperations);

    std::vector<size_t> sizes;
    sizes.reserve(numOperations);

    std::vector<std::string> allocationIDs;
    allocationIDs.reserve(numOperations);

    // Initialize random number generator for block sizes
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<size_t> sizeDist(minBlockSize, maxBlockSize);

    for (size_t i = 0; i < numOperations; ++i) {
        size_t blockSize = sizeDist(gen);

        // Time the allocation
        auto allocStart = std::chrono::high_resolution_clock::now();
        void* ptr = allocator.allocate(blockSize);
        auto allocEnd = std::chrono::high_resolution_clock::now();
        double allocTime = std::chrono::duration<double, std::micro>(allocEnd - allocStart).count(); // in microseconds

        if (ptr == nullptr) {
            std::cerr << "Allocation failed at iteration " << i << std::endl;
            break;
        }

        // Get allocation ID
        std::string allocationID = allocator.getAllocationID(ptr);

        pointers.push_back(ptr);
        sizes.push_back(blockSize);
        allocationIDs.push_back(allocationID);

        // Generate timestamp
        auto now = std::chrono::system_clock::now();
        std::time_t now_time_t = std::chrono::system_clock::to_time_t(now);
        std::tm now_tm = *std::localtime(&now_time_t);
        std::ostringstream timestampStream;
        timestampStream << std::put_time(&now_tm, "%Y-%m-%d %H:%M:%S");
        std::string timestamp = timestampStream.str();

        // Get thread ID
        std::ostringstream threadIDStream;
        threadIDStream << std::this_thread::get_id();
        std::string threadID = threadIDStream.str();

        // Get memory address
        std::string memoryAddress = allocator.getMemoryAddress(ptr);

        // Source and CallStack (placeholders)
        std::string source = __FUNCTION__; // Function name
        std::string callStack = "variableSizeBenchmark";

        // Log allocation time and fragmentation
        logger.log(timestamp, "Allocation", blockSize, allocTime, allocator.getFragmentation(),
                   source, callStack, memoryAddress, threadID, allocationID);
    }

    // Deallocate all pointers
    for (size_t i = 0; i < pointers.size(); ++i) {
        // Time the deallocation
        auto deallocStart = std::chrono::high_resolution_clock::now();
        allocator.deallocate(pointers[i]);
        auto deallocEnd = std::chrono::high_resolution_clock::now();
        double deallocTime = std::chrono::duration<double, std::micro>(deallocEnd - deallocStart).count(); // in microseconds

        // Generate timestamp
        auto now = std::chrono::system_clock::now();
        std::time_t now_time_t = std::chrono::system_clock::to_time_t(now);
        std::tm now_tm = *std::localtime(&now_time_t);
        std::ostringstream timestampStream;
        timestampStream << std::put_time(&now_tm, "%Y-%m-%d %H:%M:%S");
        std::string timestamp = timestampStream.str();

        // Get thread ID
        std::ostringstream threadIDStream;
        threadIDStream << std::this_thread::get_id();
        std::string threadID = threadIDStream.str();

        // Get memory address
        std::string memoryAddress = allocator.getMemoryAddress(pointers[i]);

        // Source and CallStack (placeholders)
        std::string source = __FUNCTION__; // Function name
        std::string callStack = "variableSizeBenchmark";

        // Log deallocation time and fragmentation
        logger.log(timestamp, "Deallocation", sizes[i], deallocTime, allocator.getFragmentation(),
                   source, callStack, memoryAddress, threadID, allocationIDs[i]);
    }

    std::cout << "Variable-Size Allocation Benchmark completed with " << numOperations << " operations." << std::endl;
}

void throughputBenchmark(CustomAllocator& allocator, size_t blockSize, double duration, DataLogger& logger) {
    std::vector<void*> pointers;
    std::vector<std::string> allocationIDs;

    // Initialize counters
    size_t allocCount = 0;
    size_t deallocCount = 0;

    // Start time
    auto startTime = std::chrono::high_resolution_clock::now();
    auto endTime = startTime + std::chrono::duration<double>(duration);

    // Run allocations and deallocations until duration is met
    while (std::chrono::high_resolution_clock::now() < endTime) {
        // Allocate memory
        auto allocStart = std::chrono::high_resolution_clock::now();
        void* ptr = allocator.allocate(blockSize);
        auto allocEnd = std::chrono::high_resolution_clock::now();
        double allocTime = std::chrono::duration<double, std::micro>(allocEnd - allocStart).count(); // in microseconds

        if (ptr != nullptr) {
            // Get allocation ID
            std::string allocationID = allocator.getAllocationID(ptr);

            pointers.push_back(ptr);
            allocationIDs.push_back(allocationID);
            allocCount++;

            // Generate timestamp
            auto now = std::chrono::system_clock::now();
            std::time_t now_time_t = std::chrono::system_clock::to_time_t(now);
            std::tm now_tm = *std::localtime(&now_time_t);
            std::ostringstream timestampStream;
            timestampStream << std::put_time(&now_tm, "%Y-%m-%d %H:%M:%S");
            std::string timestamp = timestampStream.str();

            // Get thread ID
            std::ostringstream threadIDStream;
            threadIDStream << std::this_thread::get_id();
            std::string threadID = threadIDStream.str();

            // Get memory address
            std::string memoryAddress = allocator.getMemoryAddress(ptr);

            // Source and CallStack (placeholders)
            std::string source = __FUNCTION__; // Function name
            std::string callStack = "throughputBenchmark";

            // Log allocation time and fragmentation
            logger.log(timestamp, "Allocation", blockSize, allocTime, allocator.getFragmentation(),
                       source, callStack, memoryAddress, threadID, allocationID);
        }

        // Deallocate memory if any pointers are available
        if (!pointers.empty()) {
            // Deallocate the first pointer (FIFO)
            void* ptr = pointers.front();
            std::string allocationID = allocationIDs.front();

            auto deallocStart = std::chrono::high_resolution_clock::now();
            allocator.deallocate(ptr);
            auto deallocEnd = std::chrono::high_resolution_clock::now();
            double deallocTime = std::chrono::duration<double, std::micro>(deallocEnd - deallocStart).count(); // in microseconds

            pointers.erase(pointers.begin());
            allocationIDs.erase(allocationIDs.begin());
            deallocCount++;

            // Generate timestamp
            auto now = std::chrono::system_clock::now();
            std::time_t now_time_t = std::chrono::system_clock::to_time_t(now);
            std::tm now_tm = *std::localtime(&now_time_t);
            std::ostringstream timestampStream;
            timestampStream << std::put_time(&now_tm, "%Y-%m-%d %H:%M:%S");
            std::string timestamp = timestampStream.str();

            // Get thread ID
            std::ostringstream threadIDStream;
            threadIDStream << std::this_thread::get_id();
            std::string threadID = threadIDStream.str();

            // Get memory address
            std::string memoryAddress = allocator.getMemoryAddress(ptr);

            // Source and CallStack (placeholders)
            std::string source = __FUNCTION__; // Function name
            std::string callStack = "throughputBenchmark";

            // Log deallocation time and fragmentation
            logger.log(timestamp, "Deallocation", blockSize, deallocTime, allocator.getFragmentation(),
                       source, callStack, memoryAddress, threadID, allocationID);
        }
    }

    // Deallocate any remaining pointers
    for (size_t i = 0; i < pointers.size(); ++i) {
        allocator.deallocate(pointers[i]);
        deallocCount++;
    }

    // End time
    auto actualEndTime = std::chrono::high_resolution_clock::now();
    double actualDuration = std::chrono::duration<double>(actualEndTime - startTime).count();

    // Calculate throughput
    double allocThroughput = static_cast<double>(allocCount) / actualDuration;
    double deallocThroughput = static_cast<double>(deallocCount) / actualDuration;

    // Log throughput results
    std::ostringstream summaryStream;
    summaryStream << "Throughput Benchmark Summary";
    std::string summary = summaryStream.str();

    logger.logSummary(summary, allocThroughput, deallocThroughput, allocator.getFragmentation());

    std::cout << "Throughput Benchmark completed." << std::endl;
    std::cout << "Duration: " << actualDuration << " seconds" << std::endl;
    std::cout << "Allocations: " << allocCount << " | Throughput: " << allocThroughput << " ops/sec" << std::endl;
    std::cout << "Deallocations: " << deallocCount << " | Throughput: " << deallocThroughput << " ops/sec" << std::endl;
}