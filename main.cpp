// File: main.cpp
#include "FileMerger.hpp"
#include <iostream>
#include <string>

int main(int argc, char *argv[])
{
    if (argc < 3)
    {
        std::cerr << "Usage: " << argv[0] << " <input_directory> <output_file> [batch_size]\n";
        return 1;
    }

    std::string inputDir = argv[1];
    std::string outputFile = argv[2];
    size_t batchSize = (argc >= 4) ? std::stoul(argv[3]) : 500;

    try
    {
        auto inputFiles = FileMerger::listFiles(inputDir);
        FileMerger::mergeFiles(inputFiles, outputFile, batchSize);
        std::cout << "Merge completed successfully.\n";
    }
    catch (const std::exception &e)
    {
        std::cerr << "Error: " << e.what() << "\n";
        return 1;
    }

    return 0;
}