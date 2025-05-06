# Market Data Merger System Documentation

This document provides visual representations and explanations of the Market Data Merger system architecture and components.

## 1. System Architecture

```mermaid
graph TB
    subgraph Input
        A[Market Data Files] --> B[FileReader]
        B --> C[MarketDataEntry]
    end
    
    subgraph Processing
        C --> D[Priority Queue]
        D --> E[Batch Processing]
        E --> F[Thread Pool]
    end
    
    subgraph Output
        F --> G[Merged Output File]
    end
    
    style Input fill:#f9f,stroke:#333,stroke-width:2px
    style Processing fill:#bbf,stroke:#333,stroke-width:2px
    style Output fill:#bfb,stroke:#333,stroke-width:2px
```

The system architecture shows the three main components:
- **Input**: Handles reading market data files and converting them into structured entries
- **Processing**: Manages the merging logic using priority queues and batch processing
- **Output**: Generates the final merged file with properly ordered entries

## 2. Data Structures

```mermaid
classDiagram
    class MarketDataEntry {
        +string symbol
        +string timestamp
        +double price
        +int size
        +string exchange
        +string type
        +operator>()
        +operator<()
    }
    
    class FileReader {
        +string symbol
        +ifstream file
        +MarketDataEntry currentEntry
        +bool hasMoreData
        +FileReader(symbol, filename)
        +readNextEntry()
    }
    
    class FileMerger {
        +mergeFiles(inputFiles, outputFile, batchSize)
        +listFiles(directory)
        -processBatch(batchFiles, outputFile, mutex)
    }
    
    FileReader --> MarketDataEntry : contains
    FileMerger --> FileReader : uses
```

Key data structures and their relationships:
- `MarketDataEntry`: Represents a single market data record
- `FileReader`: Manages reading and parsing of individual input files
- `FileMerger`: Coordinates the overall merging process

## 3. Process Flow

```mermaid
sequenceDiagram
    participant Main
    participant FileMerger
    participant FileReader
    participant PriorityQueue
    participant OutputFile
    
    Main->>FileMerger: mergeFiles(inputDir, outputFile)
    FileMerger->>FileMerger: listFiles()
    loop For each batch
        FileMerger->>FileReader: Create readers
        loop For each file
            FileReader->>FileReader: readNextEntry()
            FileReader->>PriorityQueue: Push entry
        end
        loop While queue not empty
            PriorityQueue->>OutputFile: Write entry
            FileReader->>FileReader: readNextEntry()
            FileReader->>PriorityQueue: Push if more data
        end
    end
    FileMerger->>Main: Complete
```

The sequence diagram shows:
1. Initial file listing and batch creation
2. File reading and entry processing
3. Priority queue management
4. Output file writing
5. Thread synchronization

## 4. Data Flow

```mermaid
flowchart LR
    subgraph Input Files
        A[CSCO.txt] --> |"2021-03-05 10:00:00.123,46.14,120,NYSE_ARCA,Ask"| D
        B[MSFT.txt] --> |"2021-03-05 10:00:00.123,228.5,120,NYSE,Ask"| D
    end
    
    subgraph Processing
        D[Priority Queue] --> E[Sort by Timestamp]
        E --> F[Sort by Symbol]
    end
    
    subgraph Output
        F --> G["Symbol,Timestamp,Price,Size,Exchange,Type
CSCO,2021-03-05 10:00:00.123,46.14,120,NYSE_ARCA,Ask
MSFT,2021-03-05 10:00:00.123,228.5,120,NYSE,Ask"]
    end
    
    style Input Files fill:#f9f,stroke:#333,stroke-width:2px
    style Processing fill:#bbf,stroke:#333,stroke-width:2px
    style Output fill:#bfb,stroke:#333,stroke-width:2px
```

Data flow illustration:
- Input files are read and parsed
- Entries are sorted by timestamp and symbol
- Final output maintains proper ordering

## 5. Threading Model

```mermaid
graph TB
    subgraph Main Thread
        A[Start] --> B[Create Batches]
        B --> C[Launch Threads]
        C --> D[Wait for Completion]
    end
    
    subgraph Worker Threads
        E[Batch 1] --> F[Process Files]
        G[Batch 2] --> H[Process Files]
        I[Batch N] --> J[Process Files]
    end
    
    subgraph Synchronization
        F --> K[Output Mutex]
        H --> K
        J --> K
    end
    
    style Main Thread fill:#f9f,stroke:#333,stroke-width:2px
    style Worker Threads fill:#bbf,stroke:#333,stroke-width:2px
    style Synchronization fill:#bfb,stroke:#333,stroke-width:2px
```

Threading model features:
- Main thread manages batch creation and thread coordination
- Worker threads process file batches in parallel
- Output mutex ensures thread-safe file writing

## Usage Example

Input file (CSCO.txt):
```
Timestamp,Price,Size,Exchange,Type
2021-03-05 10:00:00.123,46.14,120,NYSE_ARCA,Ask
2021-03-05 10:00:00.130,46.13,120,NYSE,TRADE
```

Output file (merged_output.txt):
```
Symbol,Timestamp,Price,Size,Exchange,Type
CSCO,2021-03-05 10:00:00.123,46.14,120,NYSE_ARCA,Ask
MSFT,2021-03-05 10:00:00.123,228.5,120,NYSE,Ask
CSCO,2021-03-05 10:00:00.130,46.13,120,NYSE,TRADE
```

## Key Features

1. **Efficient Processing**
   - Batch processing to manage system resources
   - Multithreaded execution for improved performance
   - Memory-efficient line-by-line processing

2. **Data Integrity**
   - Strict timestamp ordering
   - Secondary symbol ordering for same timestamps
   - Thread-safe output file writing

3. **Error Handling**
   - Robust file I/O error handling
   - Proper resource cleanup
   - Exception safety

4. **Cross-Platform**
   - Platform-independent path handling
   - Consistent file format processing
   - Standard C++17 features 