#include <iostream>
#include <unistd.h>     // For getcwd
#include <limits.h>     // For PATH_MAX
#include "data_logger.h"

DataLogger::DataLogger(const std::string& filename) {
    // Prints current working directory
    char cwd[PATH_MAX];
    if (getcwd(cwd, sizeof(cwd)) != NULL) {
        std::cout << "Current working directory: " << cwd << std::endl;
    } else {
        perror("getcwd() error");
    }

    // Opens the log file
    logFile.open(filename);
    if (logFile.is_open()) {
        std::cout << "File opened successfully: " << filename << std::endl;
        logFile << "Timestamp,Operation,BlockSize,Time,Fragmentation\n";
    } else {
        std::cerr << "Failed to open file: " << filename << std::endl;
    }
}

DataLogger::~DataLogger() {
    if (logFile.is_open()) {
        logFile.close();
    }
}

void DataLogger::log(const std::string& timestamp, const std::string& operation, size_t blockSize, double time, double fragmentation) {
    std::lock_guard<std::mutex> lock(logMutex);
    if (logFile.is_open()) {
        std::cout << "Logging data: " << timestamp << "," << operation << "," << blockSize << "," << time << "," << fragmentation << std::endl;
        logFile << timestamp << "," << operation << "," << blockSize << "," << time << "," << fragmentation << "\n";
    } else {
        std::cerr << "File not open during logging." << std::endl;
    }
}