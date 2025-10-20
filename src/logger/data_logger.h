#ifndef DATA_LOGGER_H
#define DATA_LOGGER_H

#include <fstream>
#include <mutex>
#include <string>

/**
 * @class DataLogger
 * @brief Handles logging of allocation/deallocation events and summary metrics.
 *
 * The DataLogger class provides thread-safe logging of memory allocator events
 * and summary metrics to a CSV file. It records detailed information about each
 * operation and can log summary statistics for performance benchmarks.
 */
class DataLogger {
   public:
    /**
     * @brief Constructs a new DataLogger object and initializes the log file.
     *
     * @param filename The name of the CSV file to write logs to.
     */
    DataLogger(const std::string& filename);

    /**
     * @brief Destroys the DataLogger object and closes the log file.
     */
    ~DataLogger();

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
    void log(const std::string& timestamp, const std::string& operation, size_t blockSize, double time,
             double fragmentation, const std::string& source, const std::string& callStack,
             const std::string& memoryAddress, const std::string& threadID, const std::string& allocationID);

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
    void logSummary(const std::string& summary, double allocThroughput, double deallocThroughput, double fragmentation);

   private:
    std::ofstream logFile;  ///< The output file stream for logging data.
    std::mutex logMutex;    ///< Mutex to ensure thread-safe logging.
};

#endif  // DATA_LOGGER_H
