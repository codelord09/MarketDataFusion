# Market Data File Merger

A C++ application for efficiently merging multiple market data files while maintaining timestamp ordering.

## Features

- Efficiently merges thousands of market data files
- Maintains timestamp ordering
- Handles same-timestamp entries alphabetically by symbol
- Memory-efficient processing
- Multithreaded implementation
- Unit tested with Google Test

## Requirements

- C++17 compatible compiler
- CMake (for building Google Test)
- Google Test (for running tests)

## Building

1. Install Google Test:
```bash
git clone https://github.com/google/googletest.git
cd googletest
mkdir build
cd build
cmake ..
make
sudo make install
```

2. Build the project:
```bash
make
```

## Running Tests

```bash
make test
```

## Usage

```bash
./file_merger <input_directory> <output_file> [batch_size]
```

Where:
- `input_directory`: Directory containing the input files
- `output_file`: Path to the output file
- `batch_size`: Optional parameter for number of files to process in each batch (default: 500)

## Input File Format

Input files should be CSV files with the following columns:
```
Timestamp, Price, Size, Exchange, Type
```

## Output File Format

The output file will be a CSV file with the following columns:
```
Symbol, Timestamp, Price, Size, Exchange, Type
```

## Performance Considerations

- The program processes files in batches to avoid opening too many files simultaneously
- Each batch is processed by a separate thread
- Files are read line by line to avoid loading entire files into memory
- The program maintains a priority queue to efficiently merge entries

## License

This project is licensed under the MIT License - see the LICENSE file for details. 