#ifndef DATA_LOGGER_H
#define DATA_LOGGER_H

#include <fstream>
#include <mutex>
#include <string>

class DataLogger {
public:
    DataLogger(const std::string& filename);
    ~DataLogger();

    void log(const std::string& timestamp,
             const std::string& operation,
             size_t blockSize,
             double time,
             double fragmentation,
             const std::string& source,
             const std::string& callStack,
             const std::string& memoryAddress,
             const std::string& threadID,
             const std::string& allocationID);

private:
    std::ofstream logFile;
    std::mutex logMutex;
};

#endif // DATA_LOGGER_H