// File: FileReader.cpp
#include "FileReader.hpp" // Add this include directive
#include "Entry.hpp" // ✅ Needed for Entry type
#include <sstream>
#include <vector>
#include <cctype> // For std::isspace

FileReader::FileReader(const std::string &filepath, const std::string &symbol)
    : symbol_(symbol), stream_(filepath)
{
    std::string header;
    std::getline(stream_, header);
    advance();
}

bool FileReader::hasNext() const
{
    return !currentEntry_.timestamp.empty();
}

Entry FileReader::currentEntry() const
{
    return currentEntry_;
}

void FileReader::advance()
{
    std::string line;
    if (std::getline(stream_, line))
    {
        currentEntry_ = parseLine(line);
        currentEntry_.symbol = symbol_;
    }
    else
    {
        currentEntry_ = Entry{};
    }
}

Entry FileReader::parseLine(const std::string &line)
{
    auto parts = split(line, ',');
    if (parts.size() < 5)
        return Entry{};
    Entry entry;
    entry.timestamp = trim(parts[0]);
    entry.price = std::stod(trim(parts[1]));
    entry.size = std::stoi(trim(parts[2]));
    entry.exchange = trim(parts[3]);
    entry.type = trim(parts[4]);
    return entry;
}

std::vector<std::string> FileReader::split(const std::string &s, char delimiter)
{
    std::vector<std::string> tokens;
    std::string token;
    std::istringstream tokenStream(s);
    while (std::getline(tokenStream, token, delimiter))
    {
        tokens.push_back(token);
    }
    return tokens;
}

std::string FileReader::trim(const std::string &s)
{
    auto start = s.begin();
    while (start != s.end() && std::isspace(*start))
        start++;
    auto end = s.end();
    do
    {
        end--;
    } while (std::distance(start, end) > 0 && std::isspace(*end));
    return std::string(start, end + 1);
}