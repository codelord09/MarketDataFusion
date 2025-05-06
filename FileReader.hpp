#ifndef FILE_READER_HPP
#define FILE_READER_HPP

#include <string>
#include <vector>
#include <fstream>
#include "Entry.hpp"

class FileReader
{
public:
    FileReader(const std::string &filepath, const std::string &symbol);
    bool hasNext() const;
    Entry currentEntry() const;
    void advance();

private:
    Entry parseLine(const std::string &line);
    std::vector<std::string> split(const std::string &s, char delimiter);
    std::string trim(const std::string &s);

    std::string symbol_;
    std::ifstream stream_;
    Entry currentEntry_;
};

#endif // FILE_READER_HPP
