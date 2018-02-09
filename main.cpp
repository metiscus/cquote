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
#include <termbox.h>
#include <curlpp/cURLpp.hpp>
#include <curlpp/Easy.hpp>
#include <curlpp/Options.hpp>
#include <rapidjson/rapidjson.h>
#include <rapidjson/document.h>
#include <rapidjson/prettywriter.h>
#include <unistd.h>

void do_fetch();
void update_thread();

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

struct Stock
{
    bool is_exchange;
    bool is_valid;
    std::string ticker;
    std::string name;
    std::string time;
    std::string exchange;
    float last;
    float change;
    float change_pct;
    float open;
    float low;
    float high;
    float stat_52_low;
    float stat_52_high;
    float stat_eps;
    float stat_pe;
    float stat_dividend;
    float stat_yield;
    //float stat_cap;
    std::string stat_shares;
    std::string stat_volume;
    std::string stat_avg_volume;

    Stock(const std::string& ticker_ = "")
        : is_exchange(false)
        , is_valid(false)
        , ticker(ticker_)
    {
        ;
    }
};

uint32_t update_time = 5; // seconds

std::mutex g_stocks_mutex;
std::map<std::string, Stock> g_stocks;

volatile bool stop = false;

int main(int argc, char** argv)
{
    (void)argc;
    (void)argv;

    g_stocks["EA"]    = Stock("EA");
    g_stocks["GOOGL"] = Stock("GOOGL");
    g_stocks["TSLA"]  = Stock("TSLA");
    g_stocks["AMZN"]  = Stock("AMZN");
    g_stocks["SPY"]   = Stock("SPY");
    //g_stocks[".DJI"]   = Stock(".DJI");

    std::thread update(update_thread);

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
    while (tb_peek_event(&ev, 100) >= 0) {
        switch (ev.type) {
        case TB_EVENT_KEY:
            switch (ev.key) {
            case TB_KEY_ESC:
                goto done;
                break;
            }
            break;
        case TB_EVENT_RESIZE:
            do_fetch();
            break;
        }
    }

done:
    stop = true;
    update.join();
    tb_shutdown();

    return 0;
}

float parse_float(const std::string& str)
{
    std::string s = str;
    s.erase(std::remove(s.begin(), s.end(), ','), s.end());
    return atof(s.c_str());
}

void fetch_quote(Stock& stock)
{
    using namespace rapidjson;

    try {

        // build the query
        std::string query = "https://finance.google.com/finance?q=";
        query += stock.ticker;
        query += "&output=json";

        //std::cerr<<"query: "<<query<<"\n";

        //printf_tb(1, 1, TB_YELLOW, TB_DEFAULT, "%s", query.c_str());

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
        //std::cout<<json_reply<<"\n";
        // but for some reason, what we get back isnt valid json....
        // need to remove the first two lines and the trailing ]

        json_reply = json_reply.substr(5, json_reply.length()-7);
        //std::cout<<json_reply<<"\n";

        // parse the query
        Document d;
        d.Parse(json_reply.c_str());

        //StringBuffer sb;
        //PrettyWriter<StringBuffer> writer(sb);
        //d.Accept(writer);    // Accept() traverses the DOM and generates Handler events.
        //std::cout<<sb.GetString()<<"\n";

        // update the entry
        stock.is_valid = true;
        stock.name            = d["name"].GetString();
        stock.exchange        = d["exchange"].GetString();
        stock.last            = parse_float(d["l"].GetString());
        stock.change          = parse_float(d["c"].GetString());
        stock.change_pct      = parse_float(d["cp"].GetString());
        stock.open            = parse_float(d["op"].GetString());
        stock.high            = parse_float(d["hi"].GetString());
        stock.low             = parse_float(d["lo"].GetString());
        stock.stat_52_low     = parse_float(d["lo52"].GetString());
        stock.stat_52_high    = parse_float(d["hi52"].GetString());
        stock.stat_eps        = parse_float(d["eps"].GetString());
        stock.stat_pe         = parse_float(d["pe"].GetString());
        stock.stat_dividend   = parse_float(d["ldiv"].GetString());
        stock.stat_yield      = parse_float(d["dy"].GetString());
        stock.stat_shares     = d["shares"].GetString();
        stock.stat_volume     = d["vo"].GetString();
        stock.stat_avg_volume = d["avvo"].GetString();


        //std::cerr<<"keyratios: "<<d.HasMember("keyratios")<<"\n";

        //assert(d["keyratios"]["l"].IsFloat());
        //printf_tb(1,3, TB_RED, TB_DEFAULT, "%s", stock.name.c_str());
        //printf_tb(6, 3, TB_RED, TB_DEFAULT, "%7.5f", d["l"].GetFloat());
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

    for(auto& stock : g_stocks)
    {
        fetch_quote(stock.second);
    }

    printf_tb(0, 0, TB_WHITE, TB_DEFAULT, 
        "%-8s%8s%8s%8s%8s%8s%8s%8s%8s%8s%8s",
        "Name", "Last", "Change", "Percent", "Open", "52w Hi", "52w Lo", "EPS", "PE", "Volume", "A Volume"
    );

    int row = 1;
    for(auto& itr : g_stocks)
    {
        const Stock& stock = itr.second;
        
        uint32_t fg_color = TB_WHITE;
        if(stock.change_pct > 0.5)
        {
            fg_color = TB_GREEN;
        }
        else if(stock.change_pct < -0.5)
        {
            fg_color = TB_RED;
        }

        printf_tb(0, row++, fg_color, TB_DEFAULT, "%-8s%8.2f%8.2f%7.2f%%%8.2f%8.2f%8.2f%8.2f%8.2f", 
            stock.ticker.c_str(), stock.last, stock.change, stock.change_pct, stock.open, stock.stat_52_high, stock.stat_52_low, stock.stat_eps, stock.stat_pe);
    }

    tb_present();

    g_stocks_mutex.unlock();
}

void update_thread()
{
    while(!stop)
    {
        timespec ts;
        ts.tv_sec = update_time;
        ts.tv_nsec = 0;

        do_fetch();

        nanosleep(&ts, nullptr);
    }
}