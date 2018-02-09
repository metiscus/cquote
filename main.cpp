#include <algorithm>
#include <cassert>
#include <cstdint>
#include <cstdarg>
#include <cstdio>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <map>
#include <thread>
#include <mutex>
#include <vector>
#include <termbox.h>
#include <curlpp/cURLpp.hpp>
#include <curlpp/Easy.hpp>
#include <curlpp/Options.hpp>
#include <rapidjson/rapidjson.h>
#include <rapidjson/document.h>
#include <rapidjson/prettywriter.h>

#include <set>
#include "stock.h"
#include "utility.h"

#include <unistd.h>
#include <sys/types.h>
#include <pwd.h>

void read_config();
void do_fetch();

void print_tb(const char *str, int x, int y, uint16_t fg, uint16_t bg)
{
    while (*str) {
        uint32_t uni;
        str += tb_utf8_char_to_unicode(&uni, str);
        tb_change_cell(x, y, uni, fg, bg);
        x++;
    }
}

void printf_tb(int x, int y, uint16_t fg, uint16_t bg, const char *fmt, ...)
{
    char buf[4096];
    va_list vl;
    va_start(vl, fmt);
    vsnprintf(buf, sizeof(buf), fmt, vl);
    va_end(vl);
    print_tb(buf, x, y, fg, bg);
}

uint32_t update_time = 5; // seconds
constexpr Property color_property = Property::ChangePercent;
constexpr double color_threshold  = 0.5;

std::mutex g_stocks_mutex;
std::map<std::string, Stock> g_stocks;
std::map<std::string, Stock> g_exchanges;

int main(int argc, char** argv)
{
    (void)argc;
    (void)argv;

    read_config();

    //g_stocks["EA"]    = Stock("EA");
    //g_stocks["GOOGL"] = Stock("GOOGL");
    //g_stocks["TSLA"]  = Stock("TSLA");
    //g_stocks["AMZN"]  = Stock("AMZN");
    //g_stocks["SPY"]   = Stock("SPY");
    //g_stocks["IBM"]   = Stock("IBM");

    int ret = tb_init();
    if(ret)
    {
        std::cerr<<"tb_init() failed with error code "<<ret<<"\n";
        return 1;
    }

    // init
    tb_clear();
    do_fetch();
    tb_present();
    
    struct tb_event ev;
    while (tb_peek_event(&ev, 1000 * update_time) >= 0) 
    {
        switch (ev.type) 
        {
            case TB_EVENT_KEY:
                switch (ev.key) {
                    case TB_KEY_ESC:
                        goto done;
                        break;
                    case TB_KEY_CTRL_R:
                        Stock::toggle_sort_mode();
                        break;
                }
                break;
            case TB_EVENT_RESIZE:
                //do_fetch();
                break;
        }

        do_fetch();
    }

done:
    tb_shutdown();

    return 0;
}

void fetch_quote(Stock& stock)
{
    using namespace rapidjson;

    try {
        // build the query
        std::string query = "https://finance.google.com/finance?q=";
        query += stock.get_ticker();
        query += "&output=json";

        // perform the query
        curlpp::options::Url url(query);
        curlpp::Easy myRequest;
        myRequest.setOpt(url);
        myRequest.setOpt(curlpp::options::HttpGet(true));
        
        std::stringstream ss;
        curlpp::options::WriteStream ws(&ss);
        myRequest.setOpt(ws);
        myRequest.perform();
        std::string json_reply = ss.str();

        // but for some reason, what we get back isnt valid json....
        // need to remove the first two lines and the trailing ]
        json_reply = json_reply.substr(5, json_reply.length()-7);


        // parse the query
        Document d;
        d.Parse(json_reply.c_str());

        // update the entry
        stock.set_valid();
        stock.set(Property::Name, d["name"].GetString());
        stock.set(Property::Exchange, d["exchange"].GetString());
        stock.set(Property::Last, parse_float(d["l"].GetString()));
        stock.set(Property::Change, parse_float(d["c"].GetString()));
        stock.set(Property::ChangePercent, parse_float(d["cp"].GetString()));
        stock.set(Property::Open, parse_float(d["op"].GetString()));
        stock.set(Property::High, parse_float(d["hi"].GetString()));
        stock.set(Property::Low, parse_float(d["lo"].GetString()));
        stock.set(Property::Low52, parse_float(d["lo52"].GetString()));
        stock.set(Property::High52, parse_float(d["hi52"].GetString()));
        stock.set(Property::Eps, parse_float(d["eps"].GetString()));
        stock.set(Property::Pe, parse_float(d["pe"].GetString()));
        stock.set(Property::Dividend, parse_float(d["ldiv"].GetString()));
        stock.set(Property::Yield, parse_float(d["dy"].GetString()));
        stock.set(Property::Shares, parse_float(d["shares"].GetString()));
        stock.set(Property::Volume, parse_float(d["vo"].GetString()));
        stock.set(Property::AvgVolume, parse_float(d["avvo"].GetString()));
    }
    catch ( ... )
    {
        std::cerr<<"do_fetch() failed\n";
    }
}

void do_fetch()
{
    g_stocks_mutex.lock();

    tb_clear();
    // query the various exchanges
    std::string exchange_string = "";
    for(auto& itr : g_exchanges)
    {
        fetch_quote(itr.second);
        const Stock& exch = itr.second;

        char buffer[1000];
        sprintf(buffer, "%s(%.2f %.2f %.2f%%)  ", itr.first.c_str(), exch.get(Property::Last), exch.get(Property::Change), exch.get(Property::ChangePercent));
        exchange_string += buffer;
    }

    printf_tb(0, 0, TB_YELLOW, TB_DEFAULT, exchange_string.c_str());

    std::set<Stock> stocks;
    for(auto& stock : g_stocks)
    {
        fetch_quote(stock.second);
        stocks.insert(stock.second);
    }

    printf_tb(0, 1, TB_YELLOW, TB_DEFAULT, 
        "%-8s%8s%8s%8s%8s%8s%8s%8s%8s%8s%8s",
        "Name", "Last", "Change", "Percent", "Open", "52w Hi", "52w Lo", "EPS", "PE", "Volume", "VolumeA"
    );

    int row = 2;
    for(auto& stock : stocks)
    {     
        uint32_t fg_color = TB_YELLOW;
        if(stock.get(color_property) > color_threshold)
        {
            fg_color = TB_GREEN;
        }
        else if(stock.get(color_property) < -color_threshold)
        {
            fg_color = TB_RED;
        }

        printf_tb(0, row++, fg_color, TB_DEFAULT, "%-8s%8.2f%8.2f%7.2f%%%8.2f%8.2f%8.2f%8.2f%8.2f%8.2lf%8.2lf", 
            stock.get_ticker().c_str(),
            stock.get(Property::Last), 
            stock.get(Property::Change),
            stock.get(Property::ChangePercent),
            stock.get(Property::Open),
            stock.get(Property::High52),
            stock.get(Property::Low52),
            stock.get(Property::Eps),
            stock.get(Property::Pe),
            stock.get(Property::Volume) / 100000.0,
            stock.get(Property::AvgVolume) / 100000.0
        );
    }

    tb_present();

    g_stocks_mutex.unlock();
}

void read_config()
{
    // Find the user home directory
    const char* homedir = nullptr;
    if ((homedir = getenv("HOME")) == NULL) 
    {
        homedir = getpwuid(getuid())->pw_dir;
    }

    // Check if the configuration file exists there
    uint32_t bufsize = sysconf(_SC_GETPW_R_SIZE_MAX);
    char *buffer = new char[bufsize];
    sprintf(buffer, "%s/.cquote", homedir);

    FILE *fp = fopen(buffer, "rt");
    if(!fp)
    {
        // We will write a default configuration out
        fp = fopen(buffer, "wt");
        fprintf(stderr, buffer);
        if(!fp)
        {
            exit(1);
        }

        fprintf(fp, 
            "{\n"
            "\t\"exchanges\" : [\n"
            "\t\t{\"Dow\" : \".DJI\"},\n"
            "\t\t{\"S&P500\" : \".INX\"},\n"
            "\t\t{\"NASDAQ\" : \".IXIC\"}\n"
            "\t],\n"
            "\t\"stocks\" : [\n"
            "\t\t\"EA\",\n"
            "\t\t\"GOOGL\",\n"
            "\t\t\"TSLA\",\n"
            "\t\t\"AMZN\",\n"
            "\t\t\"IBM\",\n"
            "\t\t\"MSFT\"\n"
            "\t],\n"
            "\t\"update\" : 5\n"
            "}\n"
        );
        fclose(fp);
        read_config();
        return;
    }
    fclose(fp);

    // parse the config file here
    using namespace rapidjson;
    std::ifstream config (buffer);
    std::stringstream ss;
    ss << config.rdbuf();
    config.close();

    Document reader;
    reader.Parse(ss.str().c_str());

    // Parse exchanges
    if(reader.HasMember("exchanges") && reader["exchanges"].IsArray())
    {
        const Value& a = reader["exchanges"];
        for(uint32_t i=0; i<a.Size(); ++i)
        {
            const Value& exch = a[i];
            for(Value::ConstMemberIterator itr = exch.MemberBegin(); itr != exch.MemberEnd(); ++itr)
            {
                g_exchanges[itr->name.GetString()] = Stock(itr->value.GetString());
            }
        }
    }

    // Parse stocks
    if(reader.HasMember("stocks") && reader["stocks"].IsArray())
    {
        const Value& a = reader["stocks"];
        for(uint32_t i=0; i<a.Size(); ++i)
        {
            const Value& exch = a[i];
            g_stocks[exch.GetString()] = Stock(exch.GetString());
        }
    }

    if(reader.HasMember("update") && reader["update"].IsInt())
    {
        update_time = reader["update"].GetInt();
    }
}
