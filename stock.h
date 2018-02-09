#pragma once

#include <cstdint>
#include <map>
#include <string>

enum class Property
{
    Ticker,
    Name,
    Exchange,
    Last,
    Change,
    ChangePercent,
    Open,
    Low,
    High,
    High52,
    Low52,
    Eps,
    Pe,
    Dividend,
    Yield,
    Shares,
    Volume,
    AvgVolume
};

const char* ToString(Property prop);

class Stock
{
    static Property sort_;
    static bool sort_forward_;

    bool is_exchange_;
    bool is_valid_;
    std::string ticker_;
    std::string name_;
    std::string exchange_;
    std::map<Property, double> stats_;

public:
    Stock(const std::string& ticker = "", bool is_exchange = false);
    static void set_sort(Property property);
    static void set_sort_mode(bool forward);
    static void toggle_sort_mode();

    bool operator<(const Stock& r) const;

    bool is_valid() const;

    inline void set_valid() { is_valid_ = true; }
    const std::string& get_ticker() const;
    const std::string& get_name() const;
    const std::string& get_exchange() const;
    double get(Property prop) const;

    void set(Property prop, std::string value);
    void set(Property prop, double value);
};