#include "FileMerger.hpp" // Include the correct header file
#include <fstream>
#include <iostream>
#include <filesystem>
#include <vector>
#include <string>
#include <algorithm>
#include <sstream>
#include <queue>
#include <thread>
#include <chrono>

// FileReader implementation
FileMerger::FileReader::FileReader(const std::string &symbol, const std::string &filename)
    : symbol(symbol), file(filename), hasMoreData(true)
{
    if (!file.is_open())
    {
        throw std::runtime_error("Failed to open file: " + filename);
    }
    // Skip header line
    std::string header;
    std::getline(file, header);
    hasMoreData = readNextEntry();
}

bool FileMerger::FileReader::readNextEntry()
{
    if (!file.is_open() || file.eof())
    {
        hasMoreData = false;
        return false;
    }

    std::string line;
    if (std::getline(file, line))
    {
        std::istringstream iss(line);
        std::string timestamp, priceStr, sizeStr, exchange, type;

        if (std::getline(iss, timestamp, ',') &&
            std::getline(iss, priceStr, ',') &&
            std::getline(iss, sizeStr, ',') &&
            std::getline(iss, exchange, ',') &&
            std::getline(iss, type))
        {
            currentEntry = {
                symbol,
                timestamp,
                std::stod(priceStr),
                std::stoi(sizeStr),
                exchange,
                type};
            return true;
        }
    }
    hasMoreData = false;
    return false;
}

// List all files in a directory
std::vector<std::string> FileMerger::listFiles(const std::string &directory)
{
    std::vector<std::string> files;
    std::filesystem::path dir(directory);

    for (const auto &entry : std::filesystem::directory_iterator(dir))
    {
        if (entry.is_regular_file())
        {
            // Convert path to string using generic_string() for consistent path separators
            files.push_back(entry.path().generic_string());
        }
    }

    // Sort files to ensure consistent ordering
    std::sort(files.begin(), files.end());
    return files;
}

// Process a batch of files
void FileMerger::processBatch(const std::vector<std::string> &batchFiles,
                              const std::string &outputFile,
                              std::mutex &outputMutex)
{
    // Create file readers for each file
    std::vector<std::unique_ptr<FileReader>> readers;
    for (const auto &file : batchFiles)
    {
        std::string symbol = std::filesystem::path(file).stem().string();
        readers.push_back(std::make_unique<FileReader>(symbol, file));
    }

    // Priority queue to merge entries - use greater<> to get ascending order
    auto compare = [](const FileReader *a, const FileReader *b)
    {
        const auto &entry_a = a->currentEntry;
        const auto &entry_b = b->currentEntry;

        if (entry_a.timestamp != entry_b.timestamp)
        {
            return entry_a.timestamp < entry_b.timestamp; // Reverse comparison for ascending order
        }
        return entry_a.symbol < entry_b.symbol; // Reverse comparison for ascending order
    };
    std::priority_queue<FileReader *, std::vector<FileReader *>, decltype(compare)> pq(compare);

    // Initialize priority queue
    for (auto &reader : readers)
    {
        if (reader->hasMoreData)
        {
            pq.push(reader.get());
        }
    }

    // Open output file
    std::ofstream outFile(outputFile, std::ios::app);
    if (!outFile.is_open())
    {
        throw std::runtime_error("Failed to open output file: " + outputFile);
    }

    // Write header if file is empty
    {
        std::lock_guard<std::mutex> lock(outputMutex);
        if (outFile.tellp() == 0)
        {
            outFile << "Symbol,Timestamp,Price,Size,Exchange,Type\n";
        }
    }

    // Merge entries
    while (!pq.empty())
    {
        FileReader *reader = pq.top();
        pq.pop();

        // Write the current entry
        {
            std::lock_guard<std::mutex> lock(outputMutex);
            const auto &entry = reader->currentEntry;
            outFile << entry.symbol << ","
                    << entry.timestamp << ","
                    << entry.price << ","
                    << entry.size << ","
                    << entry.exchange << ","
                    << entry.type << "\n";
        }

        // Read next entry and push back to queue if available
        if (reader->readNextEntry())
        {
            pq.push(reader);
        }
    }
}

// Main merge function
void FileMerger::mergeFiles(const std::vector<std::string> &inputFiles,
                            const std::string &outputFile,
                            size_t batchSize)
{
    if (inputFiles.empty())
    {
        throw std::runtime_error("No input files provided");
    }

    // Clear output file
    std::ofstream(outputFile, std::ios::trunc).close();

    // Process files in batches
    std::mutex outputMutex;
    std::vector<std::thread> threads;
    std::vector<std::string> currentBatch;

    for (size_t i = 0; i < inputFiles.size(); ++i)
    {
        currentBatch.push_back(inputFiles[i]);

        if (currentBatch.size() >= batchSize || i == inputFiles.size() - 1)
        {
            threads.emplace_back(processBatch, currentBatch, outputFile, std::ref(outputMutex));
            currentBatch.clear();
        }
    }

    // Wait for all threads to complete
    for (auto &thread : threads)
    {
        thread.join();
    }
}