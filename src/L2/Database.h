#pragma once


#include "Stat.h"
#include "Tick.h"
#include <fstream>
#include <stdexcept>
#include <sstream>
#include <string>
#include <vector>
#include <map>


namespace L2
{

struct Database
{
    std::string market;
    std::string date;
    std::vector<Stat> stockStats;
    std::map<int32_t, size_t> stockStatIndex;
    std::vector<Tick> ticks;

    void close()
    {
        market.clear();
        date.clear();
        stockStats.clear();
        stockStatIndex.clear();
        ticks.clear();
    }

    void open(std::string market_, std::string date_)
    {
        close();
        market = market_;
        date = date_;
    }

    void readTick()
    {
        std::string path = "/data/L2/" + market + "L2/" + date + "/stock-l2-ticks.dat";
        std::FILE *fp = std::fopen(path.c_str(), "rb");
        if (!fp) {
            perror(path.c_str());
            return;
        }
        std::fseek(fp, 0, SEEK_END);
        long pos = std::ftell(fp);
        std::rewind(fp);
        if (pos > 0) {
            ticks.resize(pos / sizeof(Tick));
            size_t n = std::fread(ticks.data(), sizeof(Tick), ticks.size(), fp);
            if (n != ticks.size()) {
                throw std::runtime_error("error reading L2 ticks");
            }
        } else {
            ticks.clear();
        }
        std::fclose(fp);
    }

    Stat &getStatic(int32_t stock)
    {
        return stockStats[stockStatIndex.at(stock)];
    }

    void readStatic()
    {
        std::string line;
        std::ifstream csv("/data/L2/" + market + "L2/" + date + "/stock-metadata.csv");
        if (!csv.is_open()) {
            throw std::runtime_error("no metadata found for " + market + " " + date);
        }

        std::getline(csv, line);

        stockStats.clear();
        stockStatIndex.clear();
        while (std::getline(csv, line)) {
            std::istringstream iss(line);
            std::string token;
            std::getline(iss, token, ',');

            Stat stat;
            std::getline(iss, token, ',');
            stat.stock = std::stoi(token);
            std::getline(iss, token, ',');
            std::getline(iss, token, ',');
            stat.openPrice = std::round(100 * std::stod(token));
            std::getline(iss, token, ',');
            stat.preClosePrice = std::round(100 * std::stod(token));
            std::getline(iss, token, ',');
            stat.highPrice = std::round(100 * std::stod(token));
            std::getline(iss, token, ',');
            stat.lowPrice = std::round(100 * std::stod(token));
            std::getline(iss, token, ',');
            stat.closePrice = std::round(100 * std::stod(token));
            std::getline(iss, token, ',');
            stat.upperLimitPrice = std::round(100 * std::stod(token));
            std::getline(iss, token, ',');
            stat.lowerLimitPrice = std::round(100 * std::stod(token));
            std::getline(iss, token, ',');
            stat.floatMV = std::stod(token) * 10000.0;

            stockStatIndex.insert({stat.stock, stockStats.size()});
            stockStats.push_back(stat);
        }
    }
};

}
