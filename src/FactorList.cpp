#include "FactorList.h"
#include <magic_enum/magic_enum.hpp>
#include <fstream>
#include <iomanip>
#include <cstdio>


void FactorList::dumpFactors(int32_t timestamp, int32_t stock)
{
    auto data = reinterpret_cast<double const *>(this);
    constexpr int32_t size = sizeof(FactorList) / sizeof(double);

    static int _ = ([] {
        std::ofstream csv("factors.csv");
        csv << "timestamp,ts_code";
        for (int32_t i = 0; i < size; ++i) {
            csv << ',' << magic_enum::enum_name(static_cast<FactorEnum>(i));
        }
        csv << '\n';
    }(), 0);

    std::ofstream csv("factors.csv", std::ios::app);
    csv << timestamp << ',' << stock;
    for (int32_t i = 0; i < size; ++i) {
        csv << ',' << std::setprecision(10) << std::scientific << data[i];
    }
    csv << '\n';
}
