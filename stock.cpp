#include "stock.h"
#include <cassert>

//Property Stock::sort_ = Property::Ticker;
Property Stock::sort_ = Property::Last;
bool Stock::sort_forward_ = true;

const char* ToString(Property prop)
{
    switch (prop)
    {
        case Property::Ticker: return "Ticker"; break;
        case Property::Name: return "Name"; break;
        case Property::Exchange: return "Exchange"; break;
        case Property::Last: return "Last"; break;
        case Property::Change: return "Change"; break;
        case Property::ChangePercent: return "ChangePercent"; break;
        case Property::Open: return "Open"; break;
        case Property::Low: return "Low"; break;
        case Property::High: return "High"; break;
        case Property::High52: return "High52"; break;
        case Property::Eps: return "Eps"; break;
        case Property::Pe: return "Pe"; break;
        case Property::Dividend: return "Dividend"; break;
        case Property::Yield: return "Yield"; break;
        case Property::Shares: return "Shares"; break;
        case Property::Volume: return "Volume"; break;
        case Property::AvgVolume: return "AvgVolume"; break;
        default: return ""; break;
    }
}

void Stock::set_sort(Property property)
{
    sort_ = property;
}

void Stock::set_sort_mode(bool forward)
{
    sort_forward_ = forward;
}

void Stock::toggle_sort_mode()
{
    sort_forward_ = !sort_forward_;
}

Stock::Stock(const std::string& ticker, bool is_exchange)
    : is_exchange_(is_exchange)
    , is_valid_(false)
    , ticker_(ticker)
    , name_("")
    , exchange_("")
{
    stats_[Property::Last] = 0.;
    stats_[Property::Change] = 0.;
    stats_[Property::ChangePercent] = 0.;
    stats_[Property::Open] = 0.;
    stats_[Property::Low] = 0.;
    stats_[Property::High] = 0.;
    stats_[Property::High52] = 0.;
    stats_[Property::Low52] = 0.;
    stats_[Property::Eps] = 0.;
    stats_[Property::Pe] = 0.;
    stats_[Property::Dividend] = 0.;
    stats_[Property::Yield] = 0.;
    stats_[Property::Shares] = 0.;
    stats_[Property::Volume] = 0.;
    stats_[Property::AvgVolume] = 0.;
}

bool Stock::operator<(const Stock& r) const
{
    bool ret = false;
    switch(sort_)
    {
        case Property::Ticker: ret = get_ticker() < r.get_ticker(); break;
        case Property::Name: ret =  get_name() < r.get_name(); break;
        default:
            ret = get(sort_) < r.get(sort_);
            break;
    }

    if(!sort_forward_) return !ret;
    return ret;
}

bool Stock::is_valid() const
{
    return is_valid_;
}

const std::string& Stock::get_ticker() const
{
    return ticker_;
}

const std::string& Stock::get_name() const
{
    return name_;
}

const std::string& Stock::get_exchange() const
{
    return exchange_;
}

double Stock::get(Property prop) const
{
    auto itr = stats_.find(prop);
    if(is_valid_ && (itr != stats_.end()))
    {
        return itr->second;
    }
    else
    {
        return 0.;
    }
}

void Stock::set(Property prop, std::string value)
{
    switch(prop)
    {
        case Property::Ticker: ticker_ = value; break;
        case Property::Name: name_ = value; break;
        case Property::Exchange: exchange_ = value; break;
        default:
            //assert(false);
            break;
    }
}

void Stock::set(Property prop, double value)
{
    switch(prop)
    {
        case Property::Last:
        case Property::Change:
        case Property::ChangePercent:
        case Property::Open:
        case Property::Low:
        case Property::High:
        case Property::High52:
        case Property::Low52:
        case Property::Eps:
        case Property::Pe:
        case Property::Dividend:
        case Property::Yield:
        case Property::Shares:
        case Property::Volume:
        case Property::AvgVolume:
            stats_[prop] = value;
            break;

        default:
            //assert(false);
            break;
    }
}