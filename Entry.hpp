// File: Entry.hpp
#ifndef ENTRY_HPP
#define ENTRY_HPP

#include <string>

struct Entry {
    std::string symbol;
    std::string timestamp;
    double price;
    int size;
    std::string exchange;
    std::string type;

    bool operator>(const Entry& other) const;
};

#endif // ENTRY_HPP