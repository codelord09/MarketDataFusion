#include "FileMerger.hpp"
#include <fstream>
#include <filesystem>
#include <sstream>
#include <cassert>
#include <iostream>
#include <algorithm>
#include <iomanip>
#include <chrono>

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

        // Create empty file
        createTestFile(
            std::filesystem::path("test_data").append("EMPTY.txt").generic_string(),
            "Timestamp,Price,Size,Exchange,Type\n");

        // Create file with invalid data
        createTestFile(
            std::filesystem::path("test_data").append("INVALID.txt").generic_string(),
            "Timestamp,Price,Size,Exchange,Type\n"
            "invalid_timestamp,invalid_price,invalid_size,invalid_exchange,invalid_type\n");

        // Create file with different timestamps
        createTestFile(
            std::filesystem::path("test_data").append("AAPL.txt").generic_string(),
            "Timestamp,Price,Size,Exchange,Type\n"
            "2021-03-05 09:59:59.999,150.25,100,NYSE,Bid\n"
            "2021-03-05 10:00:00.001,150.26,200,NYSE,Ask\n"
            "2021-03-05 10:00:00.500,150.27,300,NYSE,TRADE\n");

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

        assert(files.size() == 6);
        std::cout << "✓ File count check passed\n";

        // Use filesystem paths for proper path handling
        std::string cscoPath = std::filesystem::path("test_data").append("CSCO.txt").generic_string();
        std::string msftPath = std::filesystem::path("test_data").append("MSFT.txt").generic_string();
        std::string emptyPath = std::filesystem::path("test_data").append("EMPTY.txt").generic_string();
        std::string invalidPath = std::filesystem::path("test_data").append("INVALID.txt").generic_string();
        std::string aaplPath = std::filesystem::path("test_data").append("AAPL.txt").generic_string();

        bool foundCSCO = std::find(files.begin(), files.end(), cscoPath) != files.end();
        bool foundMSFT = std::find(files.begin(), files.end(), msftPath) != files.end();
        bool foundEmpty = std::find(files.begin(), files.end(), emptyPath) != files.end();
        bool foundInvalid = std::find(files.begin(), files.end(), invalidPath) != files.end();
        bool foundAAPL = std::find(files.begin(), files.end(), aaplPath) != files.end();

        assert(foundCSCO);
        assert(foundMSFT);
        assert(foundEmpty);
        assert(foundInvalid);
        assert(foundAAPL);

        std::cout << "✓ Found CSCO.txt\n";
        std::cout << "✓ Found MSFT.txt\n";
        std::cout << "✓ Found EMPTY.txt\n";
        std::cout << "✓ Found INVALID.txt\n";
        std::cout << "✓ Found AAPL.txt\n";
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

    void testEmptyFile()
    {
        std::cout << "\n=== Testing Empty File Handling ===\n";
        const std::string outputFile = std::filesystem::path("test_data").append("empty_output.txt").generic_string();
        std::vector<std::string> inputFiles = {
            std::filesystem::path("test_data").append("EMPTY.txt").generic_string()};

        FileMerger::mergeFiles(inputFiles, outputFile, 1);

        std::ifstream output(outputFile);
        assert(output.is_open());
        std::string line;
        std::getline(output, line); // Skip header
        assert(line == "Symbol,Timestamp,Price,Size,Exchange,Type");
        assert(!std::getline(output, line)); // Should be no more lines
        std::cout << "✓ Empty file test passed\n";
    }

    void testInvalidData()
    {
        std::cout << "\n=== Testing Invalid Data Handling ===\n";
        const std::string outputFile = std::filesystem::path("test_data").append("invalid_output.txt").generic_string();
        std::vector<std::string> inputFiles = {
            std::filesystem::path("test_data").append("INVALID.txt").generic_string()};

        try
        {
            FileMerger::mergeFiles(inputFiles, outputFile, 1);
            assert(false && "Should have thrown an exception for invalid data");
        }
        catch (const std::exception &e)
        {
            std::cout << "✓ Invalid data handling test passed (caught exception: " << e.what() << ")\n";
        }
    }

    void testDifferentTimestamps()
    {
        std::cout << "\n=== Testing Different Timestamps ===\n";
        const std::string outputFile = std::filesystem::path("test_data").append("timestamp_output.txt").generic_string();
        std::vector<std::string> inputFiles = {
            std::filesystem::path("test_data").append("AAPL.txt").generic_string(),
            std::filesystem::path("test_data").append("CSCO.txt").generic_string()};

        FileMerger::mergeFiles(inputFiles, outputFile, 1);

        std::ifstream output(outputFile);
        assert(output.is_open());
        std::string line;
        std::getline(output, line); // Skip header

        // Verify entries are in correct timestamp order
        std::vector<std::string> expectedEntries = {
            "AAPL,2021-03-05 09:59:59.999,150.25,100,NYSE,Bid",
            "AAPL,2021-03-05 10:00:00.001,150.26,200,NYSE,Ask",
            "CSCO,2021-03-05 10:00:00.123,46.14,120,NYSE_ARCA,Ask",
            "CSCO,2021-03-05 10:00:00.130,46.13,120,NYSE,TRADE",
            "AAPL,2021-03-05 10:00:00.500,150.27,300,NYSE,TRADE"};

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
            std::cout << "✓ Entry " << (i + 1) << " matches expected timestamp order\n";
        }
        std::cout << "Different timestamps test passed!\n";
    }

    void testLargeBatchSize()
    {
        std::cout << "\n=== Testing Large Batch Size ===\n";
        const std::string outputFile = std::filesystem::path("test_data").append("batch_output.txt").generic_string();
        std::vector<std::string> inputFiles = {
            std::filesystem::path("test_data").append("CSCO.txt").generic_string(),
            std::filesystem::path("test_data").append("MSFT.txt").generic_string(),
            std::filesystem::path("test_data").append("AAPL.txt").generic_string()};

        // Test with a large batch size
        FileMerger::mergeFiles(inputFiles, outputFile, 1000);

        std::ifstream output(outputFile);
        assert(output.is_open());
        std::string line;
        std::getline(output, line); // Skip header
        assert(line == "Symbol,Timestamp,Price,Size,Exchange,Type");

        // Count total entries
        size_t entryCount = 0;
        while (std::getline(output, line))
        {
            entryCount++;
        }
        assert(entryCount == 7); // 2 from CSCO + 2 from MSFT + 3 from AAPL
        std::cout << "✓ Large batch size test passed\n";
    }

    void testErrorHandling()
    {
        std::cout << "\n=== Testing Error Handling ===\n";

        // Test non-existent file
        try
        {
            std::vector<std::string> nonExistentFiles = {"non_existent_file.txt"};
            FileMerger::mergeFiles(nonExistentFiles, "output.txt", 1);
            assert(false && "Should have thrown an exception for non-existent file");
        }
        catch (const std::exception &e)
        {
            std::cout << "✓ Non-existent file handling test passed\n";
        }

        // Test invalid output path
        try
        {
            std::vector<std::string> validFiles = {
                std::filesystem::path("test_data").append("CSCO.txt").generic_string()};
            FileMerger::mergeFiles(validFiles, "/invalid/path/output.txt", 1);
            assert(false && "Should have thrown an exception for invalid output path");
        }
        catch (const std::exception &e)
        {
            std::cout << "✓ Invalid output path handling test passed\n";
        }
    }

    void testLargeDataset()
    {
        std::cout << "\n=== Testing Large Dataset ===\n";

        // Create multiple files with overlapping timestamps
        const int NUM_FILES = 10;
        const int ENTRIES_PER_FILE = 1000;
        std::vector<std::string> symbols = {"AAPL", "MSFT", "GOOGL", "AMZN", "META", "TSLA", "NVDA", "AMD", "INTC", "CSCO"};

        // Create files with overlapping timestamps
        for (int i = 0; i < NUM_FILES; ++i)
        {
            std::stringstream content;
            content << "Timestamp,Price,Size,Exchange,Type\n";

            // Generate entries with overlapping timestamps
            for (int j = 0; j < ENTRIES_PER_FILE; ++j)
            {
                // Create timestamps that overlap between files
                std::string timestamp = "2021-03-05 10:00:00.";
                timestamp += std::to_string(j * 10 + i); // Offset each file's timestamps

                // Generate random price between 100 and 1000
                double price = 100.0 + (rand() % 900) + (rand() % 100) / 100.0;

                // Generate random size between 100 and 10000
                int size = 100 + (rand() % 9900);

                // Alternate between exchanges
                std::string exchange = (j % 2 == 0) ? "NYSE" : "NASDAQ";

                // Alternate between types
                std::string type = (j % 3 == 0) ? "Bid" : (j % 3 == 1) ? "Ask"
                                                                       : "TRADE";

                content << timestamp << "," << std::fixed << std::setprecision(2)
                        << price << "," << size << "," << exchange << "," << type << "\n";
            }

            std::string filename = std::filesystem::path("test_data")
                                       .append(symbols[i] + "_large.txt")
                                       .generic_string();
            createTestFile(filename, content.str());
        }

        // Test merging with different batch sizes
        std::vector<size_t> batchSizes = {1, 100, 1000, 10000};

        for (size_t batchSize : batchSizes)
        {
            std::cout << "\nTesting with batch size: " << batchSize << "\n";

            const std::string outputFile = std::filesystem::path("test_data")
                                               .append("large_output_" + std::to_string(batchSize) + ".txt")
                                               .generic_string();

            // Get all input files
            auto inputFiles = FileMerger::listFiles("test_data");
            inputFiles.erase(
                std::remove_if(
                    inputFiles.begin(),
                    inputFiles.end(),
                    [](const std::string &s)
                    {
                        return s.find("large_output_") != std::string::npos;
                    }),
                inputFiles.end());

            // Measure execution time
            auto start = std::chrono::high_resolution_clock::now();

            // Merge files
            FileMerger::mergeFiles(inputFiles, outputFile, batchSize);

            auto end = std::chrono::high_resolution_clock::now();
            auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);

            std::cout << "Merge completed in " << duration.count() << "ms\n";

            // Verify output
            std::ifstream output(outputFile);
            assert(output.is_open());

            std::string line;
            std::getline(output, line); // Skip header
            assert(line == "Symbol,Timestamp,Price,Size,Exchange,Type");

            // Count total entries
            size_t entryCount = 0;
            std::string prevTimestamp;
            bool isFirstLine = true;

            while (std::getline(output, line))
            {
                entryCount++;

                // Verify timestamp ordering
                std::string currentTimestamp = line.substr(line.find(",") + 1, 23); // Extract timestamp
                if (!isFirstLine)
                {
                    assert(currentTimestamp >= prevTimestamp && "Entries not in timestamp order");
                }
                prevTimestamp = currentTimestamp;
                isFirstLine = false;
            }

            // Verify total count
            assert(entryCount == NUM_FILES * ENTRIES_PER_FILE);
            std::cout << "✓ Verified " << entryCount << " entries\n";
            std::cout << "✓ Verified timestamp ordering\n";
        }

        std::cout << "Large dataset test passed!\n";
    }

public:
    void runTests()
    {
        try
        {
            setup();
            testFileListing();
            testFileMerging();
            testEmptyFile();
            testInvalidData();
            testDifferentTimestamps();
            testLargeBatchSize();
            testErrorHandling();
            testLargeDataset();
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