#include "FileMerger.hpp"
#include <fstream>
#include <filesystem>
#include <sstream>
#include <cassert>
#include <iostream>
#include <algorithm>
#include <iomanip>

class FileMergerTest
{
protected:
    void setup()
    {
        std::cout << "\n=== Setting up test environment ===\n";

        // Clean up any existing test files
        if (std::filesystem::exists("test_data"))
        {
            std::filesystem::remove_all("test_data");
            std::cout << "Cleaned up existing test directory\n";
        }

        // Create test directory
        std::filesystem::create_directory("test_data");
        std::cout << "Created test directory\n";

        // Create test files using filesystem paths for proper conversion
        createTestFile(
            std::filesystem::path("test_data").append("CSCO.txt").generic_string(),
            "Timestamp,Price,Size,Exchange,Type\n"
            "2021-03-05 10:00:00.123,46.14,120,NYSE_ARCA,Ask\n"
            "2021-03-05 10:00:00.130,46.13,120,NYSE,TRADE\n");

        createTestFile(
            std::filesystem::path("test_data").append("MSFT.txt").generic_string(),
            "Timestamp,Price,Size,Exchange,Type\n"
            "2021-03-05 10:00:00.123,228.5,120,NYSE,Ask\n"
            "2021-03-05 10:00:00.133,228.5,120,NYSE,TRADE\n");
        std::cout << "Test environment setup complete.\n";
    }

    void cleanup()
    {
        std::cout << "\n=== Cleaning up test environment ===\n";
        // Clean up test files
        if (std::filesystem::exists("test_data"))
        {
            std::filesystem::remove_all("test_data");
            std::cout << "Removed test directory\n";
        }
        std::cout << "Cleanup complete.\n";
    }

    void createTestFile(const std::string &filename, const std::string &content)
    {
        std::ofstream file(filename);
        if (!file.is_open())
        {
            throw std::runtime_error("Failed to create test file: " + filename);
        }
        file << content;
        std::cout << "Created test file: " << filename << "\n";
    }

    void testFileListing()
    {
        std::cout << "\n=== Testing File Listing ===\n";
        auto files = FileMerger::listFiles("test_data");
        std::cout << "Found " << files.size() << " files in test directory.\n";
        for (const auto &file : files)
        {
            std::cout << "  - " << file << "\n";
        }

        assert(files.size() == 2);
        std::cout << "✓ File count check passed\n";

        // Use filesystem paths for proper path handling
        std::string cscoPath = std::filesystem::path("test_data").append("CSCO.txt").generic_string();
        std::string msftPath = std::filesystem::path("test_data").append("MSFT.txt").generic_string();

        bool foundCSCO = std::find(files.begin(), files.end(), cscoPath) != files.end();
        bool foundMSFT = std::find(files.begin(), files.end(), msftPath) != files.end();

        assert(foundCSCO);
        assert(foundMSFT);

        std::cout << "✓ Found CSCO.txt\n";
        std::cout << "✓ Found MSFT.txt\n";
        std::cout << "File listing test passed!\n";
    }

    void testFileMerging()
    {
        std::cout << "\n=== Testing File Merging ===\n";
        const std::string outputFile = std::filesystem::path("test_data").append("output.txt").generic_string();

        // Get input files
        auto inputFiles = FileMerger::listFiles("test_data");
        std::cout << "Processing " << inputFiles.size() << " input files.\n";

        inputFiles.erase(
            std::remove_if(
                inputFiles.begin(),
                inputFiles.end(),
                [](const std::string &s)
                { return s.find("output.txt") != std::string::npos; }),
            inputFiles.end());

        // Merge files
        std::cout << "Merging files with batch size 1...\n";
        FileMerger::mergeFiles(inputFiles, outputFile, 1);

        // Verify output
        std::ifstream output(outputFile);
        assert(output.is_open());
        std::cout << "✓ Output file created successfully\n";

        std::string line;
        std::getline(output, line); // Skip header
        assert(line == "Symbol,Timestamp,Price,Size,Exchange,Type");
        std::cout << "✓ Header format correct\n";

        // Verify entries
        std::vector<std::string> expectedEntries = {
            "CSCO,2021-03-05 10:00:00.123,46.14,120,NYSE_ARCA,Ask",
            "MSFT,2021-03-05 10:00:00.123,228.5,120,NYSE,Ask",
            "CSCO,2021-03-05 10:00:00.130,46.13,120,NYSE,TRADE",
            "MSFT,2021-03-05 10:00:00.133,228.5,120,NYSE,TRADE"};

        for (size_t i = 0; i < expectedEntries.size(); ++i)
        {
            std::getline(output, line);
            if (line != expectedEntries[i])
            {
                std::cerr << "Line mismatch at entry " << (i + 1) << ":\n";
                std::cerr << "Expected: " << expectedEntries[i] << "\n";
                std::cerr << "Got:      " << line << "\n";
                assert(false);
            }
            std::cout << "✓ Entry " << (i + 1) << " matches expected format\n";
        }

        std::cout << "File merging test passed!\n";
    }

public:
    void runTests()
    {
        try
        {
            setup();
            testFileListing();
            testFileMerging();
            cleanup();
            std::cout << "\n=== All tests passed successfully! ===\n";
        }
        catch (const std::exception &e)
        {
            std::cerr << "\nTest failed: " << e.what() << "\n";
            cleanup();
            throw;
        }
    }
};

int main()
{
    try
    {
        FileMergerTest test;
        test.runTests();
        return 0;
    }
    catch (...)
    {
        std::cerr << "\nTest suite failed!\n";
        return 1;
    }
}