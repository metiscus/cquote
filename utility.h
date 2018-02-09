#pragma once

#include <algorithm>
#include <cstdlib>
#include <string>

inline double parse_float(const std::string& str)
{
    std::string s = str;
    // Remove the commas
    s.erase(std::remove(s.begin(), s.end(), ','), s.end());

    // Remove trailing T, B, M, K and multiply later
    double scale = 1.0;
    switch(s.back())
    {
        case 'K': scale = 1e3; break;
        case 'M': scale = 1e6; break;
        case 'B': scale = 1e9; break;
        case 'T': scale = 1e12; break;

        default:
            break;
    }

    // Remove the trailing multiplier
    if(scale > 1.0)
    {
        s = s.substr(0, s.length()-1);
    }

    return scale * atof(s.c_str());
}