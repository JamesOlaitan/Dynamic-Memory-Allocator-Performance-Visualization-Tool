#include "data_logger.h"

#include <limits.h>  // For PATH_MAX
#include <unistd.h>  // For getcwd

#include <iomanip>  // For std::put_time
#include <iostream>
#include <sstream>  // For std::ostringstream

/**
 * @brief Constructs a new DataLogger object and initializes the log file.
 *
 * @param filename The name of the CSV file to write logs to.
 */
DataLogger::DataLogger(const std::string& filename) {
    // Print current working directory
    char cwd[PATH_MAX];
    if (getcwd(cwd, sizeof(cwd)) != NULL) {
        std::cout << "Current working directory: " << cwd << std::endl;
    } else {
        perror("getcwd() error");
    }

    // Checks if filename is empty
    std::string actualFilename = filename.empty() ? "performance_data.csv" : filename;

    if (actualFilename.empty()) {
        actualFilename = "performance_data.csv";
        std::cerr << "No filename provided. Using default 'performance_data.csv'." << std::endl;
    }

    // Opens the log file
    logFile.open(actualFilename, std::ios::out | std::ios::app);
    if (logFile.is_open()) {
        // Check if the file is empty to write headers
        logFile.seekp(0, std::ios::end);
        if (logFile.tellp() == 0) {
            std::cout << "File opened successfully: " << actualFilename << std::endl;
            logFile << "Timestamp,Operation,BlockSize,Time,Fragmentation,Source,CallStack,MemoryAddress,ThreadID,"
                       "AllocationID\n";
        } else {
            std::cout << "File opened successfully: " << actualFilename << std::endl;
        }
    } else {
        std::cerr << "Failed to open file: " << actualFilename << std::endl;
    }
}

/**
 * @brief Destroys the DataLogger object and closes the log file.
 */
DataLogger::~DataLogger() {
    if (logFile.is_open()) {
        logFile.close();
    }
}

/**
 * @brief Logs an allocation or deallocation event.
 *
 * Records detailed information about each memory allocation or deallocation
 * operation, including timing and fragmentation metrics.
 *
 * @param timestamp The timestamp of the operation in "YYYY-MM-DD HH:MM:SS" format.
 * @param operation The type of operation ("Allocation" or "Deallocation").
 * @param blockSize The size of the memory block in bytes.
 * @param time The time taken for the operation in microseconds.
 * @param fragmentation The current memory fragmentation percentage.
 * @param source The source function or module initiating the operation.
 * @param callStack The call stack trace leading to the operation.
 * @param memoryAddress The memory address allocated or deallocated.
 * @param threadID The ID of the thread performing the operation.
 * @param allocationID The unique identifier for the allocation.
 */
void DataLogger::log(const std::string& timestamp, const std::string& operation, size_t blockSize, double time,
                     double fragmentation, const std::string& source, const std::string& callStack,
                     const std::string& memoryAddress, const std::string& threadID, const std::string& allocationID) {
    std::lock_guard<std::mutex> lock(logMutex);
    if (logFile.is_open()) {
        // Log to console
        std::cout << "Logging data: " << timestamp << "," << operation << "," << blockSize << "," << time << ","
                  << fragmentation << "," << source << "," << callStack << "," << memoryAddress << "," << threadID
                  << "," << allocationID << std::endl;

        // Log to file
        logFile << timestamp << "," << operation << "," << blockSize << "," << time << "," << fragmentation << ","
                << source << "," << callStack << "," << memoryAddress << "," << threadID << "," << allocationID << "\n";
    } else {
        std::cerr << "File not open during logging." << std::endl;
    }
}

/**
 * @brief Logs summary metrics for performance benchmarks.
 *
 * Records aggregated performance metrics such as allocation and deallocation
 * throughput alongside memory fragmentation. This method appends a summary
 * entry to the same CSV log file with the operation type set to "Summary".
 *
 * @param summary A descriptive summary of the benchmark.
 * @param allocThroughput Allocation throughput in operations per second.
 * @param deallocThroughput Deallocation throughput in operations per second.
 * @param fragmentation Current memory fragmentation percentage.
 */
void DataLogger::logSummary(const std::string& summary, double allocThroughput, double deallocThroughput,
                            double fragmentation) {
    std::lock_guard<std::mutex> lock(logMutex);
    if (logFile.is_open()) {
        // Current timestamp
        auto now = std::chrono::system_clock::now();
        std::time_t now_time_t = std::chrono::system_clock::to_time_t(now);
        std::tm now_tm;
#if defined(_WIN32) || defined(_WIN64)
        localtime_s(&now_tm, &now_time_t);
#else
        localtime_r(&now_time_t, &now_tm);
#endif
        std::ostringstream timestampStream;
        timestampStream << std::put_time(&now_tm, "%Y-%m-%d %H:%M:%S");
        std::string timestamp = timestampStream.str();

        // Log to console
        std::cout << "Logging summary: " << timestamp << ","
                  << "Summary"
                  << ","
                  << "0"
                  << "," << allocThroughput << "," << deallocThroughput << "," << fragmentation << "," << summary << ","
                  << ","  // MemoryAddress
                  << ","  // ThreadID
                  << ","  // AllocationID
                  << "\n";

        // Log to file
        logFile << timestamp << ","
                << "Summary"
                << ","  // Operation
                << "0"
                << ","                       // BlockSize (0 indicates summary)
                << allocThroughput << ","    // Time (alloc throughput)
                << deallocThroughput << ","  // Fragmentation (dealloc throughput)
                << fragmentation << ","      // Source (fragmentation)
                << summary << ","            // CallStack (summary description)
                << ","                       // MemoryAddress
                << ","                       // ThreadID
                << ","                       // AllocationID
                << "\n";
    } else {
        std::cerr << "File not open during summary logging." << std::endl;
    }
}