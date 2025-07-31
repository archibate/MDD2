#include "FactorList.h"
#include "heatZone.h"
#include "generatedModels.h"
#include <magic_enum/magic_enum.hpp>
#include <fstream>
#include <iomanip>
#include <cstdio>


COLD_ZONE void FactorList::dumpFactors(int32_t timestamp, int32_t stock) const
{
    auto data = reinterpret_cast<double const *>(this);
    constexpr int32_t size = sizeof(FactorList) / sizeof(double);

    static int _ = ([] {
        std::ofstream csv("factors.csv");
        csv << "timestamp,ts_code";
        for (int32_t i = 0; i < size; ++i) {
            csv << ',' << magic_enum::enum_name(static_cast<FactorEnum>(i));
        }
        csv << ",result_classification,result_regression,result_prediction";
        csv << '\n';
    }(), 0);

    std::ofstream csv("factors.csv", std::ios::app);
    csv << timestamp << ',' << stock;
    for (int32_t i = 0; i < size; ++i) {
        csv << ',' << std::setprecision(10) << std::scientific << data[i];
    }

    double cls = predictModelClassification(rawFactors);
    double reg = predictModelRegression(rawFactors);
    double pred = cls > 0 && reg > 0 ? (cls + reg) * 0.5 : 0;
    csv << ',' << cls;
    csv << ',' << reg;
    csv << ',' << pred;

    csv << '\n';
}
