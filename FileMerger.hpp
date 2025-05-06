// File: FileMerger.hpp
#pragma once

#include <string>
#include <vector>
#include <queue>
#include <mutex>
#include <memory>
#include <fstream>
#include <thread>
#include <condition_variable>

class FileMerger
{
public:
    // Structure to hold a single market data entry
    struct MarketDataEntry
    {
        std::string symbol;
        std::string timestamp;
        double price;
        int size;
        std::string exchange;
        std::string type;

        bool operator>(const MarketDataEntry &other) const
        {
            if (timestamp != other.timestamp)
            {
                return timestamp > other.timestamp;
            }
            return symbol > other.symbol;
        }

        bool operator<(const MarketDataEntry &other) const
        {
            if (timestamp != other.timestamp)
            {
                return timestamp < other.timestamp;
            }
            return symbol < other.symbol;
        }
    };

    // Structure to manage a single input file
    struct FileReader
    {
        std::string symbol;
        std::ifstream file;
        MarketDataEntry currentEntry;
        bool hasMoreData;

        FileReader(const std::string &symbol, const std::string &filename);
        bool readNextEntry();
    };

    // Merge files from input directory to output file
    static void mergeFiles(const std::vector<std::string> &inputFiles,
                           const std::string &outputFile,
                           size_t batchSize = 500);

    // List all files in a directory
    static std::vector<std::string> listFiles(const std::string &directory);

private:
    // Process a batch of files
    static void processBatch(const std::vector<std::string> &batchFiles,
                             const std::string &outputFile,
                             std::mutex &outputMutex);
};
