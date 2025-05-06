// File: Entry.cpp
#include "Entry.hpp"
#include <algorithm>

bool Entry::operator>(const Entry& other) const {
    if (timestamp != other.timestamp)
        return timestamp > other.timestamp;
    return symbol > other.symbol;
}