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
 * @brief Performs sequential allocation and deallocation of memory blocks.
 * @param allocator Reference to the CustomAllocator instance.
 * @param blockSize Size of each memory block to allocate.
 * @param numOperations Number of allocation/deallocation operations to perform.
 * @param logger Reference to the DataLogger instance.
 */
void sequentialAllocationTest(CustomAllocator& allocator, size_t blockSize, size_t numOperations, DataLogger& logger);

/**
 * @brief Performs random allocation and deallocation of memory blocks.
 * @param allocator Reference to the CustomAllocator instance.
 * @param minBlockSize Minimum size of memory blocks to allocate.
 * @param maxBlockSize Maximum size of memory blocks to allocate.
 * @param numOperations Number of allocation/deallocation operations to perform.
 * @param logger Reference to the DataLogger instance.
 */
void randomAllocationTest(CustomAllocator& allocator, size_t minBlockSize, size_t maxBlockSize, size_t numOperations, DataLogger& logger);

/**
 * @brief Performs allocation and deallocation with mixed block sizes to simulate real-world usage.
 * @param allocator Reference to the CustomAllocator instance.
 * @param sizeDistribution A vector containing the distribution of block sizes.
 * @param numOperations Number of allocation/deallocation operations to perform.
 * @param logger Reference to the DataLogger instance.
 */
void mixedSizesTest(CustomAllocator& allocator, const std::vector<size_t>& sizeDistribution, size_t numOperations, DataLogger& logger);

/**
 * @brief Entry point for the allocator test.
 * @param argc Number of command-line arguments.
 * @param argv Array of command-line argument strings.
 * @return Exit code.
 */
int main(int argc, char* argv[]) {
    std::cout << "Running allocator_tests.cpp main function." << std::endl;
    cxxopts::Options options("allocator_tests", "Dynamic Memory Allocator Performance Tests");

    options.add_options()
        ("test", "Test type [sequential|random|mixed]", cxxopts::value<std::string>()->default_value("sequential"))
        ("num-ops", "Number of operations", cxxopts::value<size_t>()->default_value("1000"))
        ("block-size", "Block size in bytes", cxxopts::value<size_t>()->default_value("64"))
        ("min-block-size", "Minimum block size in bytes", cxxopts::value<size_t>()->default_value("32"))
        ("max-block-size", "Maximum block size in bytes", cxxopts::value<size_t>()->default_value("512"))
        ("min-order", "Minimum order", cxxopts::value<size_t>()->default_value("5"))
        ("max-order", "Maximum order", cxxopts::value<size_t>()->default_value("20"))
        ("output-file", "Path to output file", cxxopts::value<std::string>()->default_value("performance_data.csv"))
        ("help", "Print help");

    auto result = options.parse(argc, argv);

    if (result.count("help")) {
        std::cout << options.help() << std::endl;
        return 0;
    }

    // Extract options
    std::string testType = result["test"].as<std::string>();
    size_t numOperations = result["num-ops"].as<size_t>();
    size_t blockSize = result["block-size"].as<size_t>();
    size_t minBlockSize = result["min-block-size"].as<size_t>();
    size_t maxBlockSize = result["max-block-size"].as<size_t>();
    size_t minOrder = result["min-order"].as<size_t>();
    size_t maxOrder = result["max-order"].as<size_t>();
    std::string outputFile = result["output-file"].as<std::string>();

    // Initialize the DataLogger
    DataLogger logger(outputFile);

    // Initialize the allocator
    CustomAllocator allocator(minOrder, maxOrder);

    // Run the selected test
    if (testType == "sequential") {
        sequentialAllocationTest(allocator, blockSize, numOperations, logger);
    } else if (testType == "random") {
        randomAllocationTest(allocator, minBlockSize, maxBlockSize, numOperations, logger);
    } else if (testType == "mixed") {
        std::vector<size_t> sizeDistribution = {32, 64, 128, 256, 512, 1024};
        mixedSizesTest(allocator, sizeDistribution, numOperations, logger);
    } else {
        std::cerr << "Invalid test type specified.\n";
        return 1;
    }

    return 0;
}

void sequentialAllocationTest(CustomAllocator& allocator, size_t blockSize, size_t numOperations, DataLogger& logger) {
    std::vector<void*> pointers;
    pointers.reserve(numOperations);

    std::vector<std::string> allocationIDs;
    allocationIDs.reserve(numOperations);

    for (size_t i = 0; i < numOperations; ++i) {
        // Time the allocation
        auto allocStart = std::chrono::high_resolution_clock::now();
        void* ptr = allocator.allocate(blockSize);
        auto allocEnd = std::chrono::high_resolution_clock::now();
        double allocTime = std::chrono::duration<double>(allocEnd - allocStart).count();

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
        std::string callStack = "sequentialAllocationTest";

        // Log allocation time and fragmentation
        logger.log(timestamp, "Allocation", blockSize, allocTime, allocator.getFragmentation(),
                   source, callStack, memoryAddress, threadID, allocationID);
    }

    // Deallocate in the same order
    for (size_t i = 0; i < pointers.size(); ++i) {
        // Time the deallocation
        auto deallocStart = std::chrono::high_resolution_clock::now();
        allocator.deallocate(pointers[i]);
        auto deallocEnd = std::chrono::high_resolution_clock::now();
        double deallocTime = std::chrono::duration<double>(deallocEnd - deallocStart).count();

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
        std::string callStack = "sequentialAllocationTest";

        // Log deallocation time and fragmentation
        logger.log(timestamp, "Deallocation", blockSize, deallocTime, allocator.getFragmentation(),
                   source, callStack, memoryAddress, threadID, allocationIDs[i]);
    }

    std::cout << "Sequential Allocation Test completed with " << numOperations << " operations." << std::endl;
}

void randomAllocationTest(CustomAllocator& allocator, size_t minBlockSize, size_t maxBlockSize, size_t numOperations, DataLogger& logger) {
    std::vector<void*> pointers;
    std::vector<size_t> sizes;
    std::vector<std::string> allocationIDs;
    pointers.reserve(numOperations);
    sizes.reserve(numOperations);
    allocationIDs.reserve(numOperations);

    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<size_t> sizeDist(minBlockSize, maxBlockSize);
    std::uniform_int_distribution<int> opDist(0, 1); // 0: allocate, 1: deallocate

    for (size_t i = 0; i < numOperations; ++i) {
        int operation = opDist(gen);

        if (operation == 0 || pointers.empty()) {
            // Allocation
            size_t blockSize = sizeDist(gen);
            auto allocStart = std::chrono::high_resolution_clock::now();
            void* ptr = allocator.allocate(blockSize);
            auto allocEnd = std::chrono::high_resolution_clock::now();
            double allocTime = std::chrono::duration<double>(allocEnd - allocStart).count();

            if (ptr != nullptr) {
                pointers.push_back(ptr);
                sizes.push_back(blockSize);

                // Get allocation ID
                std::string allocationID = allocator.getAllocationID(ptr);
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
                std::string callStack = "randomAllocationTest";

                // Log allocation time and fragmentation
                logger.log(timestamp, "Allocation", blockSize, allocTime, allocator.getFragmentation(),
                           source, callStack, memoryAddress, threadID, allocationID);
            } else {
                std::cerr << "Allocation failed at iteration " << i << std::endl;
            }
        } else if (!pointers.empty()) {
            // Deallocation
            std::uniform_int_distribution<size_t> indexDist(0, pointers.size() - 1);
            size_t index = indexDist(gen);

            auto deallocStart = std::chrono::high_resolution_clock::now();
            allocator.deallocate(pointers[index]);
            auto deallocEnd = std::chrono::high_resolution_clock::now();
            double deallocTime = std::chrono::duration<double>(deallocEnd - deallocStart).count();

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
            std::string memoryAddress = allocator.getMemoryAddress(pointers[index]);

            // Source and CallStack (placeholders)
            std::string source = __FUNCTION__; // Function name
            std::string callStack = "randomAllocationTest";

            // Log deallocation time and fragmentation
            logger.log(timestamp, "Deallocation", sizes[index], deallocTime, allocator.getFragmentation(),
                       source, callStack, memoryAddress, threadID, allocationIDs[index]);

            // Remove from vectors
            pointers.erase(pointers.begin() + index);
            sizes.erase(sizes.begin() + index);
            allocationIDs.erase(allocationIDs.begin() + index);
        }
    }

    // Deallocate remaining pointers
    for (size_t i = 0; i < pointers.size(); ++i) {
        allocator.deallocate(pointers[i]);
    }

    std::cout << "Random Allocation Test completed with " << numOperations << " operations." << std::endl;
}

void mixedSizesTest(CustomAllocator& allocator, const std::vector<size_t>& sizeDistribution, size_t numOperations, DataLogger& logger) {
    std::vector<void*> pointers;
    std::vector<size_t> sizes;
    std::vector<std::string> allocationIDs;
    pointers.reserve(numOperations);
    sizes.reserve(numOperations);
    allocationIDs.reserve(numOperations);

    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<size_t> sizeIndexDist(0, sizeDistribution.size() - 1);
    std::uniform_int_distribution<int> opDist(0, 1); // 0: allocate, 1: deallocate

    for (size_t i = 0; i < numOperations; ++i) {
        int operation = opDist(gen);

        if (operation == 0 || pointers.empty()) {
            // Allocation
            size_t sizeIndex = sizeIndexDist(gen);
            size_t blockSize = sizeDistribution[sizeIndex];
            auto allocStart = std::chrono::high_resolution_clock::now();
            void* ptr = allocator.allocate(blockSize);
            auto allocEnd = std::chrono::high_resolution_clock::now();
            double allocTime = std::chrono::duration<double>(allocEnd - allocStart).count();

            if (ptr != nullptr) {
                pointers.push_back(ptr);
                sizes.push_back(blockSize);

                // Get allocation ID
                std::string allocationID = allocator.getAllocationID(ptr);
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
                std::string callStack = "mixedSizesTest";

                // Log allocation time and fragmentation
                logger.log(timestamp, "Allocation", blockSize, allocTime, allocator.getFragmentation(),
                           source, callStack, memoryAddress, threadID, allocationID);
            } else {
                std::cerr << "Allocation failed at iteration " << i << std::endl;
            }
        } else if (!pointers.empty()) {
            // Deallocation
            std::uniform_int_distribution<size_t> indexDist(0, pointers.size() - 1);
            size_t index = indexDist(gen);

            auto deallocStart = std::chrono::high_resolution_clock::now();
            allocator.deallocate(pointers[index]);
            auto deallocEnd = std::chrono::high_resolution_clock::now();
            double deallocTime = std::chrono::duration<double>(deallocEnd - deallocStart).count();

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
            std::string memoryAddress = allocator.getMemoryAddress(pointers[index]);

            // Source and CallStack (placeholders)
            std::string source = __FUNCTION__; // Function name
            std::string callStack = "mixedSizesTest";

            // Log deallocation time and fragmentation
            logger.log(timestamp, "Deallocation", sizes[index], deallocTime, allocator.getFragmentation(),
                       source, callStack, memoryAddress, threadID, allocationIDs[index]);

            // Remove from vectors
            pointers.erase(pointers.begin() + index);
            sizes.erase(sizes.begin() + index);
            allocationIDs.erase(allocationIDs.begin() + index);
        }
    }

    // Deallocate remaining pointers
    for (size_t i = 0; i < pointers.size(); ++i) {
        allocator.deallocate(pointers[i]);
    }

    std::cout << "Mixed Sizes Test completed with " << numOperations << " operations." << std::endl;
}