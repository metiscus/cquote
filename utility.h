#pragma once

#include <algorithm>
#include <cstdlib>
#include <string>

inline double parse_float(const std::string& str)
{
    std::string s = str;
    s.erase(std::remove(s.begin(), s.end(), ','), s.end());
    return atof(s.c_str());
}