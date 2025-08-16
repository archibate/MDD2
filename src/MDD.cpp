#include "MDD.h"
#include "MDS.h"
#include "OES.h"
#include "SPSC.h"
#include "constants.h"
#include "threadAffinity.h"
#include "clockMonotonic.h"
#include "DailyFactorFile.h"
#include "generatedModels.h"
#include "IIRState.h"
#include "FactorList.h"
#include "dateTime.h"
#include "radixSort.h"
#include "simdAlgo.h"
#include "timestamp.h"
#include "securityId.h"
#include "BuyRequest.h"
#include "Reflect.h"
#include "heatZone.h"
#include "generatedReflect.h"
#include "strXele.h"
#include <thread>
#include <fstream>
#include <cmath>
#include <algorithm>
#include <absl/container/btree_map.h>
// #include <absl/container/flat_hash_set.h>
#include <tsl/robin_set.h>
#include <nlohmann/json.hpp>
#include <spdlog/spdlog.h>
#include <magic_enum/magic_enum_format.hpp>
#include <emmintrin.h>

namespace
{

#if SZ
const uint64_t g_szOffsetTransactTime = static_cast<uint64_t>(getToday()) * UINT64_C(1'00'00'00'000);
#endif

struct State
{
#if SH
    uint32_t upperLimitPrice1000{};
#endif
#if SZ
    uint64_t upperLimitPrice10000{};
    int64_t upRemainQty100{};
    tsl::robin_set<uint32_t> upSellOrderIds;
#endif
};

struct alignas(64) Compute
{
    struct Snapshot
    {
        int32_t lastPrice;
        int32_t numTrades;
        int64_t volume;
        int64_t amount;
    };

    struct FState
    {
        uint32_t nextTickTimestamp{};
        Snapshot currSnapshot{};
        std::vector<Snapshot> snapshots;
        std::unique_ptr<IIRState> iirState{};
    };

    struct BState
    {
        uint32_t nextTickTimestamp{};
        Snapshot currSnapshot{};
        size_t oldSnapshotsCount{};
        std::unique_ptr<IIRState> iirState{};
    };

    int32_t stockCode{};
    int32_t upperLimitPrice{};
    int32_t preClosePrice{};
    int32_t upperLimitPriceApproach{};
    bool approachingLimitUp{};

    int32_t openPrice{};
    int64_t openVolume{};

    uint64_t wantSign{};

    FState fState;
    BState bState;

    struct UpSell {
        int32_t price;
        int32_t quantity;
    };

    absl::btree_map<uint64_t, UpSell> upSellOrders;

#if RECORD_FACTORS
    std::unique_ptr<FactorList[]> factorListCache;
#endif
};

using TickRing = spsc_ring<MDS::Tick, 0x40000>; // 16MB
using WantSign = std::atomic<uint64_t>;

std::array<std::jthread, kChannelCount> g_computeThreads;

int64_t g_initTotalFund; // owned by mds thread
int64_t g_availableFund; // owned by mds thread

int32_t *g_stockCodes; // constant, owned by all
size_t g_numStocks; // constant, owned by all
int32_t *g_prevLimitUpStockCodes; // constant, owned by all
size_t g_numPrevLimitUpStocks; // constant, owned by all

alignas(4096) std::array<int16_t, 0x2000> g_stockIdLut; // constant, owned by all

double *g_prevLimitUpReturns; // owned by mds_snap thread
double g_prevLimitUpMeanReturn; // shared by mds thread and mds_snap thread


State *g_stockStates; // owned by mds thread
uint8_t *g_idToChannelLut; // owned by mds thread
BuyRequest *g_buyRequests; // owned by mds thread

Compute *g_stockComputes; // owned by compute thread [c:c]
FactorList *g_stockFactors; // owned by compute thread [c:c]

WantSign *g_wantSigns; // shared by compute thread [c:c] and mds thread
TickRing *g_tickRings; // shared by compute thread [c] and mds thread

enum Intent : uint8_t
{
    NotSure,
    DontBuy,
    WantBuy,
};

// SH 600xxx 601xxx 603xxx 605xxx
// SZ 000xxx 001xxx 002xxx 003xxx
uint16_t linearizeStockIdRaw(int32_t stock)
{
    uint32_t u = static_cast<uint32_t>(stock);
#if SH
    u -= UINT32_C(600000);
#endif
    if (u & ~UINT32_C(0x1FFF)) {
        u = UINT32_C(0x1FFF);
    }
    return static_cast<uint16_t>(u);
}

int32_t linearizeStockId(int32_t stock)
{
    uint32_t u = static_cast<uint32_t>(stock);
#if SH
    u -= UINT32_C(600000);
#endif
    if (u & ~UINT32_C(0x1FFF)) {
        return -1;
    }
    return static_cast<int32_t>(g_stockIdLut[static_cast<uint16_t>(u)]);
}

void initStockArrays()
{
    for (int32_t s = 0; s < g_stockIdLut.size(); ++s) {
        g_stockIdLut[s] = -1;
    }
    for (int32_t i = 0; i < g_numStocks; ++i) {
        g_stockIdLut[linearizeStockIdRaw(g_stockCodes[i])] = i;
    }

    g_idToChannelLut = new uint8_t[g_numStocks];
    memset(g_idToChannelLut, -1, g_numStocks * sizeof(uint8_t));
    for (int32_t channel = 0; channel < kChannelCount; ++channel) {
        const int32_t startId = (g_numStocks * channel) / kChannelCount;
        const int32_t stopId = (g_numStocks * (channel + 1)) / kChannelCount;
        for (int32_t id = startId; id != stopId; ++id) {
            g_idToChannelLut[id] = static_cast<uint8_t>(channel);
        }
    }

    g_stockStates = new State[g_numStocks] {};
    g_stockComputes = new Compute[g_numStocks] {};
    g_wantSigns = new WantSign[g_numStocks] {};
    g_tickRings = new TickRing[kChannelCount] {};
    g_buyRequests = new OES::ReqOrder[g_numStocks] {};
}

void parseDailyConfig(const char *config)
{
    SPDLOG_INFO("reading config file: {}", config);

    int32_t date = 0;
    std::string factorFile;
    try {
        nlohmann::json json;
        std::ifstream(config) >> json;
        SPDLOG_DEBUG("config json: {}", json.dump());

        if (json.contains("date")) {
            date = json["date"];
        }
        if (json.contains("factor_file")) {
            factorFile = json["factor_file"];
        }
    } catch (std::exception const &e) {
        SPDLOG_ERROR("config json parse failed: {}", e.what());
        throw;
    }

    SPDLOG_INFO("daily start: market={} date={}", MARKET_NAME, date);
#if !REPLAY
    if (int32_t today = getToday(); date != today) {
        SPDLOG_WARN("config file date not today: fileDate={} todayDate={}", date, today);
    }
#endif

    if (factorFile.empty()) {
#if REPLAY
        factorFile = REPLAY_DATA_PATH "/daily_csv/mdd2_factors_" MARKET_NAME_LOWER "_" + std::to_string(date) + ".bin";
#else
        SPDLOG_ERROR("no factor file provided in config");
        throw std::runtime_error("no factor file provided in config");
#endif
    }
    std::ifstream facFile(factorFile, std::ios::binary);
    if (!facFile.is_open()) {
        SPDLOG_ERROR("cannot open daily factors for market={} date={} factorFile=[{}]", MARKET_NAME, date, factorFile);
        throw std::runtime_error("cannot open daily factors");
    }
    DailyFactorFileHeader header{};
    facFile.read((char *)&header, sizeof(header));

    constexpr int32_t kFileVersion = 250722;
    if (header.fileVersion != kFileVersion) {
        SPDLOG_ERROR("daily factor file version mismatch: fileVersion={} expectVersion={}",
                     header.fileVersion, kFileVersion);
        throw std::runtime_error("daily factor file version mismatch");
    }
    if (header.marketID != MARKET_ID) {
        SPDLOG_ERROR("daily factor file market id mismatch: fileMarketId={} expectMarketId={}",
                     header.marketID, MARKET_ID);
        throw std::runtime_error("daily factor file market id mismatch");
    }
    if (header.factorCount != FactorEnum::kMaxFactors) {
        SPDLOG_ERROR("daily factor file factor count mismatch: fileFactorCount={} expectFactorCount={}",
                     header.factorCount, FactorEnum::kMaxFactors);
        throw std::runtime_error("daily factor file factor count mismatch");
    }
    if (header.factorDtypeSize != sizeof(FactorList::ScalarType)) {
        SPDLOG_ERROR("daily factor file factor dtype mismatch: fileDtypeSize={} expectDtypeSize={}",
                     header.factorDtypeSize, sizeof(FactorList::ScalarType));
        throw std::runtime_error("daily factor file factor dtype mismatch");
    }

    if (header.today != date) {
        SPDLOG_ERROR("daily factor file today date mismatch: fileToday={} expectToday={}",
                     header.today, date);
        throw std::runtime_error("daily factor file today date mismatch");
    }
    if (!header.stockCount) {
        SPDLOG_ERROR("no stock subscription found in daily factor file for market={} date={}", MARKET_NAME, date);
        throw std::runtime_error("no stock subscription found in daily factor file");
    }
    SPDLOG_DEBUG("daily factor file contains numStocks={} numPrevLimitUp={} numFactors={}", header.stockCount, header.prevLimitUpCount, header.factorCount);

    g_stockCodes = new int32_t[header.stockCount]{};
    g_numStocks = header.stockCount;
    facFile.read((char *)g_stockCodes, g_numStocks * sizeof(int32_t));
    if (!facFile.good()) {
        SPDLOG_ERROR("failed to read all stock subscribes");
        throw std::runtime_error("failed to read all stock subscribes");
    }

    g_numPrevLimitUpStocks = header.prevLimitUpCount;
    g_prevLimitUpStockCodes = new int32_t[g_numPrevLimitUpStocks]{};
    facFile.read((char *)g_prevLimitUpStockCodes, g_numPrevLimitUpStocks * sizeof(int32_t));
    if (!facFile.good()) {
        SPDLOG_ERROR("failed to read all prev-limit-up stocks");
        throw std::runtime_error("failed to read all prev-limit-up stocks");
    }
    g_prevLimitUpReturns = new double[g_numPrevLimitUpStocks];
    memset(g_prevLimitUpReturns, -1, sizeof(double) * g_numPrevLimitUpStocks);

    static_assert(FactorEnum::kMaxFactors * sizeof(FactorList::ScalarType) == sizeof(FactorList));
    g_stockFactors = new FactorList[g_numStocks]{};
    facFile.read((char *)g_stockFactors, g_numStocks * sizeof(FactorList));
    if (!facFile.good()) {
        SPDLOG_ERROR("failed to read all stock factors");
        throw std::runtime_error("failed to read all stock factors");
    }

    facFile.close();
    initStockArrays();
}

HEAT_ZONE_SNAPSHOT void stepSnapshot(Compute &compute)
{
    compute.fState.iirState->addVolumeTick(compute.fState.currSnapshot.volume);
    compute.fState.snapshots.push_back(compute.fState.currSnapshot);
    compute.fState.currSnapshot.numTrades = 0;
    compute.fState.currSnapshot.volume = 0;
    compute.fState.currSnapshot.amount = 0;
}

HEAT_ZONE_SNAPSHOT void stepSnapshotUntil(Compute &compute, uint32_t timestamp)
{
    if (timestamp > compute.fState.nextTickTimestamp) {
        stepSnapshot(compute);
        while (timestamp > (compute.fState.nextTickTimestamp
            = timestampAdvance100ms(compute.fState.nextTickTimestamp))) {
            stepSnapshot(compute);
        }
    }
}

HEAT_ZONE_COMPUTE void computeMomentum(Compute &compute, FactorList &factorList)
{
    for (int32_t m = 0; m < kMomentumDurations.size(); ++m) {
        auto &factor = factorList.momentum[m];

        if (compute.fState.snapshots.size() >= kMomentumDurations[m] + 1) {
            {
                double sum = 0;
                double sumSqr = 0;
                double prevPrice = compute.fState.snapshots[0].lastPrice;
                for (int32_t t = 1; t < kMomentumDurations[m] + 1; ++t) {
                    double currPrice = compute.fState.snapshots[t].lastPrice;
                    double changeRate = currPrice / prevPrice - 1.0;
                    sum += changeRate;
                    sumSqr += changeRate * changeRate;
                    prevPrice = currPrice;
                }
                double mean = sum * (1.0 / kMomentumDurations[m]);
                double variance = sumSqr * (1.0 / kMomentumDurations[m]) - mean * mean;
                double std = std::sqrt(std::max(0.0, variance));

                factor.openMean = mean;
                factor.openStd = std;
                factor.openZScore = std > 1e-10 ? mean / std : 0.0;
            }

            {
                double sum = 0;
                double sumSqr = 0;
                double prevPrice = compute.fState.snapshots[compute.fState.snapshots.size() - kMomentumDurations[m] - 1].lastPrice;
                for (int32_t t = compute.fState.snapshots.size() - kMomentumDurations[m]; t < compute.fState.snapshots.size(); ++t) {
                    double currPrice = compute.fState.snapshots[t].lastPrice;
                    double changeRate = currPrice / prevPrice - 1.0;
                    sum += changeRate;
                    sumSqr += changeRate * changeRate;
                    prevPrice = currPrice;
                }
                double mean = sum * (1.0 / kMomentumDurations[m]);
                double variance = sumSqr * (1.0 / kMomentumDurations[m]) - mean * mean;
                double std = std::sqrt(std::max(0.0, variance));

                factor.highMean = mean;
                factor.highStd = std;
                factor.highZScore = std > 1e-10 ? mean / std : 0.0;
            }

            factor.diffMean = factor.openMean - factorList.momentum[m].highMean;
            factor.diffZScore = factor.openZScore - factor.highZScore;

        } else {
            std::memset(&factor, -1, sizeof(factor));
        }
    }
}

HEAT_ZONE_COMPUTE void computeSkewKurt(double &skew, double &kurt, double *first, double *last)
{
    size_t n = last - first;
    if (n <= 3) {
        skew = kurt = NAN;
        return;
    }

    double sum = 0;
    for (double *p = first; p != last; ++p) {
        sum += *p;
    }
    double mean = sum / n;

    double variance = 0;
    double skewness = 0;
    double kurtosis = 0;
    for (double *p = first; p != last; ++p) {
        double value = *p - mean;
        double valueSqr = value * value;
        variance += valueSqr;
        skewness += valueSqr * value;
        kurtosis += valueSqr * valueSqr;
    }

    if (variance <= 0) {
        skew = kurt = 0;
        return;
    }
    double var = variance / n;
    double std = std::sqrt(var);

    skew = skewness / (n * var * std);
    kurt = kurtosis / (n * var * var) - 3;

    skew = skew * std::sqrt(n * (n - 1)) / (n - 2);
    kurt = (kurt * (n + 1) + 6) * (n - 1) / ((n - 2) * (n - 3));
}

HEAT_ZONE_COMPUTE void computeVolatility(Compute &compute, FactorList &factorList)
{
    std::memset(&factorList.volatility, -1, sizeof(factorList.volatility));

    size_t n = compute.fState.snapshots.size() / 5 + 1;
    thread_local std::vector<int64_t> volumeGather;
    thread_local std::vector<int64_t> amountGather;
    thread_local std::vector<double> vwap;

    volumeGather.clear();
    amountGather.clear();
    volumeGather.resize(n);
    amountGather.resize(n);
    amountGather[0] = compute.openVolume * compute.openPrice;
    volumeGather[0] = compute.openVolume;
    for (size_t i = 0; i < compute.fState.snapshots.size(); ++i) {
        volumeGather[i / 5] += compute.fState.snapshots[i].volume;
        amountGather[i / 5] += compute.fState.snapshots[i].amount;
    }

    vwap.resize(n);
    double lastVWAP = compute.openPrice;
    for (size_t i = 0; i < n; ++i) {
        if (volumeGather[i] != 0) {
            lastVWAP = static_cast<double>(amountGather[i]) / static_cast<double>(volumeGather[i]);
        }
        vwap[i] = lastVWAP;
    }

    double p0 = compute.preClosePrice;
    double p4 = p0 * 1.04;
    double p5 = p0 * 1.05;
    double p6 = p0 * 1.06;
    double p7 = p0 * 1.07;
    double p8 = p0 * 1.08;
    double p9 = p0 * 1.09;

    struct LCSRange
    {
        int64_t amount{};
        int64_t volume{};
        int32_t count{};

        int32_t start{0};
        int32_t stop{-1};
        int32_t curr{-1};
        int32_t last{-1};

        void lcsAdd(int32_t i)
        {
            if (curr == -1 || i != last + 1) {
                curr = i;
            } else if (i - curr >= stop - start) {
                start = curr;
                stop = i;
            }
            last = i;
        }
    };

    LCSRange ranges[3];
    for (size_t i = 0; i < n; ++i) {
        double v = vwap[i];
        if (v >= p4 && v <= p7) {
            ranges[0].lcsAdd(i);
            ranges[0].amount += amountGather[i];
            ranges[0].volume += volumeGather[i];
            ++ranges[0].count;
        }
        if (v >= p5 && v <= p8) {
            ranges[1].lcsAdd(i);
            ranges[1].amount += amountGather[i];
            ranges[1].volume += volumeGather[i];
            ++ranges[1].count;
        }
        if (v >= p6 && v <= p9) {
            ranges[2].lcsAdd(i);
            ranges[2].amount += amountGather[i];
            ranges[2].volume += volumeGather[i];
            ++ranges[2].count;
        }
    }

    double floatMV = factorList.floatMV;
    int64_t totalAmount = 0;
    int64_t totalVolume = 0;
    for (size_t i = 0; i < n; ++i) {
        totalAmount += amountGather[i];
        totalVolume += volumeGather[i];
    }

    computeSkewKurt(factorList.volatility[0].vwapSkew, factorList.volatility[0].vwapKurt, vwap.data(), vwap.data() + vwap.size());
    factorList.volatility[2].vwapSkew = factorList.volatility[1].vwapSkew = factorList.volatility[0].vwapSkew;
    factorList.volatility[2].vwapKurt = factorList.volatility[1].vwapKurt = factorList.volatility[0].vwapKurt;

    for (int32_t l = 0; l < 3; ++l) {
        auto &range = ranges[l];
        auto &factor = factorList.volatility[l];

        if (range.start <= range.stop) {
            double sumSqr = 0;
            double sumPosSqr = 0;
            double sumPosSqrSqr = 0;
            int32_t nPositive = 0;
            for (int32_t i = std::max(1, range.start); i <= range.stop; ++i) {
                double r = vwap[i] / vwap[i - 1] - 1.0;
                double r2 = r * r;
                sumSqr += r2;
                if (r > 0) {
                    sumPosSqr += r2;
                    sumPosSqrSqr += r2 * r2;
                    ++nPositive;
                }
            }

            if (sumSqr != 0) {
                factor.upVolume = std::sqrt(sumPosSqr);
                factor.upRatio = sumPosSqr / sumSqr;
                if (nPositive > 1) {
                    factor.upStd = std::sqrt(std::max(0.0, (sumPosSqrSqr - (sumPosSqr * sumPosSqr) / (nPositive - 1)) / (nPositive - 1)));
                }
            }

            factor.time = range.count * 0.25;
            factor.consecTime = (range.stop - range.start + 1) * 0.25;
            factor.turnover = static_cast<double>(range.amount) / floatMV * 0.01;
            factor.amountRatio = static_cast<double>(range.amount) / totalAmount;
            factor.volumeRatio = static_cast<double>(range.volume) / totalVolume;

            int64_t lcsAmount = 0;
            int64_t lcsVolume = 0;
            for (int32_t i = range.start; i <= range.stop; ++i) {
                lcsAmount += amountGather[i];
                lcsVolume += volumeGather[i];
            }
            factor.consecTurnover = static_cast<double>(lcsAmount) / floatMV * 0.01;
            factor.consecAmountRatio = static_cast<double>(lcsAmount) / totalAmount;
            factor.consecVolumeRatio = static_cast<double>(lcsVolume) / totalVolume;

            computeSkewKurt(factor.consecVwapSkew, factor.consecVwapKurt, vwap.data() + range.start, vwap.data() + range.stop + 1);
        }
    }
}

HEAT_ZONE_COMPUTE void computeKaiyuan(Compute &compute, FactorList &factorList)
{
    struct Transaction
    {
        float meanAmount;
        float sumAmount;
        float changeRate;

        bool operator<(Transaction const &right) const
        {
            return meanAmount < right.meanAmount;
        }
    };

    thread_local std::vector<Transaction> transactions;
    transactions.clear();
    transactions.reserve(compute.fState.snapshots.size() / 10);
    float prevPrice = compute.fState.snapshots[0].lastPrice;
    for (size_t i = 1; i < compute.fState.snapshots.size(); ++i) {
        if (compute.fState.snapshots[i].numTrades != 0) {
            float currPrice = compute.fState.snapshots[i].lastPrice;
            float r = currPrice / prevPrice - 1.0f;
            prevPrice = currPrice;

            float a = static_cast<float>(compute.fState.snapshots[i].amount);
            transactions.push_back({a / compute.fState.snapshots[i].numTrades, a, r});
        }
    }

    std::memset(&factorList.kaiyuan, -1, sizeof(factorList.kaiyuan));
    if (!transactions.empty()) {
        radixSort<8, 4, sizeof(float), offsetof(Transaction, meanAmount), sizeof(Transaction)>(transactions.data(), transactions.size());

        auto m = transactions.size();
        auto it096 = transactions.begin() + static_cast<size_t>(std::ceil(0.096 * m));
        auto it096f = transactions.begin() + static_cast<size_t>(std::floor(0.096 * m));
        auto it10 = transactions.begin() + static_cast<size_t>(std::ceil(0.10 * m));
        auto it10f = transactions.begin() + static_cast<size_t>(std::floor(0.10 * m));
        auto it50 = transactions.begin() + static_cast<size_t>(std::ceil(0.50 * m));
        auto it70 = transactions.begin() + static_cast<size_t>(std::ceil(0.70 * m));
        auto it90 = transactions.begin() + static_cast<size_t>(std::ceil(0.90 * m));
        auto it96 = transactions.begin() + static_cast<size_t>(std::ceil(0.96 * m));

        float A0 = transactions.front().meanAmount;
        float A100 = transactions.back().meanAmount;
        float A096 = it096->meanAmount;
        float A10 = it10->meanAmount;
        float A096f = it096f->meanAmount;
        float A10f = it10f->meanAmount;
        float A96 = it96->meanAmount;
        if (A100 > A0) {
            factorList.kaiyuan.quantile = (0.5f * (A10 + A10f) - A0) / (A100 - A0);
        }
        if (A96 > A0) {
            factorList.kaiyuan.trimmedQuantile = (0.5f * (A096 + A096f) - A0) / (A96 - A0);
        }

        double sumT = 0;
        double sumA = 0;
        double sumTA = 0;
        double sumT2 = 0;
        double sumA2 = 0;
        for (auto it = transactions.begin(); it != it96; ++it) {
            sumT += it->meanAmount;
            sumA += it->sumAmount;
            sumTA += it->meanAmount * it->sumAmount;
            sumT2 += it->meanAmount * it->meanAmount;
            sumA2 += it->sumAmount * it->sumAmount;
        }
        double size = it96 - transactions.begin();
        double numerator = sumTA * size - sumT * sumA;
        double denominatorX = sumT2 * size - sumT * sumT;
        double denominatorY = sumA2 * size - sumA * sumA;
        factorList.kaiyuan.trimmedCorrelation = numerator / std::sqrt(std::max(0.0, denominatorX * denominatorY));

        for (auto it = it96; it != transactions.end(); ++it) {
            sumT += it->meanAmount;
            sumA += it->sumAmount;
            sumTA += it->meanAmount * it->sumAmount;
            sumT2 += it->meanAmount * it->meanAmount;
            sumA2 += it->sumAmount * it->sumAmount;
        }
        size = transactions.size();
        numerator = sumTA * size - sumT * sumA;
        denominatorX = sumT2 * size - sumT * sumT;
        denominatorY = sumA2 * size - sumA * sumA;
        factorList.kaiyuan.correlation = numerator / std::sqrt(std::max(0.0, denominatorX * denominatorY));

        double sr = 0;
        for (auto it = it90; it != transactions.end(); ++it) {
            sr += it->changeRate;
        }
        factorList.kaiyuan.signalReturn10 = sr;
        for (auto it = it70; it != it90; ++it) {
            sr += it->changeRate;
        }
        factorList.kaiyuan.signalReturn30 = sr;
        for (auto it = it50; it != it70; ++it) {
            sr += it->changeRate;
        }
        factorList.kaiyuan.signalReturn50 = sr;
    }
}

HEAT_ZONE_COMPUTE void computeFutureWantBuy(std::vector<int32_t> &approachStockIds)
{
    for (int32_t id: approachStockIds) {
        auto &compute = g_stockComputes[id];
        compute.wantSign = static_cast<uint32_t>(timestampLinear(compute.fState.nextTickTimestamp)) / 100U;
    }
    static_assert(kWantSignLookAhead <= 16 && kWantSignLookAhead > 0);
    for (uint32_t offset = 0; offset < kWantSignLookAhead; ++offset) {
        for (int32_t id: approachStockIds) {
            auto &compute = g_stockComputes[id];
            if (compute.fState.snapshots.empty()) [[unlikely]] {
                continue;
            }
            if (compute.upperLimitPrice == 0 || compute.openPrice == compute.upperLimitPrice) [[unlikely]] {
                continue;
            }

            uint32_t timestamp = timestampAdvance(compute.fState.nextTickTimestamp, offset * 100);

            compute.bState.nextTickTimestamp = compute.fState.nextTickTimestamp;
            compute.bState.currSnapshot = compute.fState.currSnapshot;
            compute.bState.oldSnapshotsCount = compute.fState.snapshots.size();
            *compute.bState.iirState = *compute.fState.iirState;
            compute.bState.iirState.swap(compute.fState.iirState);

            stepSnapshotUntil(compute, timestamp);

            for (auto const &[sellOrderId, sell]: compute.upSellOrders) {
                int64_t q64 = sell.quantity;
                ++compute.fState.currSnapshot.numTrades;
                compute.fState.currSnapshot.volume += q64;
                compute.fState.currSnapshot.amount += sell.price * q64;
            }
            compute.fState.currSnapshot.lastPrice = compute.upperLimitPrice;
            stepSnapshot(compute);

            auto &factorList = g_stockFactors[id];
            computeMomentum(compute, factorList);
            computeVolatility(compute, factorList);
            computeKaiyuan(compute, factorList);
            compute.fState.iirState->finalCompute(factorList.crowdind);
            factorList.prevUMeanReturn = g_prevLimitUpMeanReturn;

            bool wantBuy = predictModel(factorList.rawFactors);
            compute.wantSign |= static_cast<uint64_t>(wantBuy ? WantBuy : DontBuy) << (32 + offset * 2);
            g_wantSigns[id].store(compute.wantSign, std::memory_order_relaxed);
            // SPDLOG_INFO("computed model: stock={:06d} timestamp={} offset={} wantBuy={}", compute.stockCode, timestamp, offset, wantBuy);

#if RECORD_FACTORS
            compute.factorListCache[offset] = factorList;
#endif

            compute.fState.nextTickTimestamp = compute.bState.nextTickTimestamp;
            compute.fState.currSnapshot = compute.bState.currSnapshot;
            compute.fState.snapshots.resize(compute.bState.oldSnapshotsCount);
            compute.fState.iirState.swap(compute.bState.iirState);
        }
    }
}

HEAT_ZONE_REQORDER Intent checkWantBuyAtTimestamp(int32_t id, uint32_t timestamp)
{
    uint32_t linearTimestamp = (static_cast<uint32_t>(timestampLinear(timestamp)) + 90U) / 100U;
    uint64_t wantSign = g_wantSigns[id].load(std::memory_order_relaxed);
    uint32_t offset = (linearTimestamp - static_cast<uint32_t>(wantSign)) / 100U;
    Intent intent;
    if (offset >= kWantSignLookAhead) [[unlikely]] {
        intent = NotSure;
    } else {
        intent = static_cast<Intent>(static_cast<uint8_t>(wantSign >> (32 + offset * 2)) & 0b11);
    }
    return intent;
}

HEAT_ZONE_REQORDER void sendBuyRequest(int32_t id)
{
#if SPLIT_ORDER
    OES::sendReqOrderBatch(g_buyRequests[id]);
#else
    OES::sendReqOrder(g_buyRequests[id]);
#endif
}

COLD_ZONE void reportTickRingOverflow(int32_t channel)
{
    SPDLOG_WARN("tick ring overflow: channel={}", channel);
}

COLD_ZONE void reportTickRingInvalidCh(int32_t channel)
{
    SPDLOG_WARN("tick ring invalid channel: channel={}", channel);
}

HEAT_ZONE_TICK void pushTickToRing(int32_t id, MDS::Tick &tick)
{
    uint8_t ch = g_idToChannelLut[id];
    if (ch == 0xff) [[unlikely]] {
        reportTickRingInvalidCh(ch);
        return;
    }
    if (!g_tickRings[ch].write_one(tick)) [[unlikely]] {
        reportTickRingOverflow(ch);
    }
}

void unsubscribeStock(int32_t id)
{
    auto &state = g_stockStates[id];
#if SH
    state.upperLimitPrice1000 = 0;
#endif
#if SZ
    state.upperLimitPrice10000 = 0;
#endif
}

void logStop(int32_t id, uint32_t timestamp, Intent intent)
{
    int32_t stock = g_stockCodes[id];
    MDS::Tick endSign{};
#if SH
    endSign.tickMergeSse.tickType = 0;
    fmtSecurityId(endSign.tickMergeSse.securityID, stock);
    endSign.tickMergeSse.tickTime = timestamp / 10;
    endSign.tickMergeSse.tickBSFlag = static_cast<char>(intent);
#endif
#if SZ
    endSign.tradeSz.messageType = 0;
    fmtSecurityId(endSign.tradeSz.securityID, stock);
    endSign.tradeSz.transactTime = g_szOffsetTransactTime + timestamp;
    endSign.tradeSz.execType = static_cast<uint8_t>(intent);
#endif
    pushTickToRing(id, endSign);

    SPDLOG_DEBUG("detected limit up: stock={:06d} timestamp={} intent={}", g_stockCodes[id], timestamp, intent);
    unsubscribeStock(id);
}

}

HEAT_ZONE_TICK void MDD::handleTick(MDS::Tick &tick)
{
    int32_t stock = securityId(tick.commonHead.securityID);
    int32_t id = linearizeStockId(stock);
    if (id == -1) [[unlikely]] {
        return;
    }
    auto &state = g_stockStates[id];

#if SH
    bool limitUp = tick.tickMergeSse.tickType == 'A'
        && tick.tickMergeSse.tickBSFlag == '0'
        && tick.tickMergeSse.price == state.upperLimitPrice1000
        && tick.tickMergeSse.tickTime >= 9'30'00'00
        && tick.tickMergeSse.tickTime < 14'57'00'00;
    if (limitUp) {
        uint32_t timestamp = tick.tickMergeSse.tickTime * 10;
        auto intent = checkWantBuyAtTimestamp(id, timestamp);
        if (intent == WantBuy || ALWAYS_BUY) [[likely]] {
            sendBuyRequest(id);
        }

        logStop(id, timestamp, intent);
        return;
    }
#endif

#if SZ
    if (tick.messageType == XeleCompat::MSG_TYPE_ORDER_SZ) {
        if (tick.orderSz.side == '2'
            && tick.orderSz.orderType == '2'
            && tick.orderSz.price == state.upperLimitPrice10000) {
            state.upRemainQty100 += tick.orderSz.qty;
            state.upSellOrderIds.insert(tick.orderSz.applSeqNum);
        }
    } else {
        // assert(tick.messageType == NescForesight::MSG_TYPE_TRADE_SZ);
        if (tick.tradeSz.execType == 0x1) {
            if (state.upSellOrderIds.contains(tick.tradeSz.offerapplSeqnum)) {
                state.upRemainQty100 -= tick.tradeSz.tradeQty;
            }

        } else {
            // assert(tick.tradeSz.execType == 0x2);
            if (tick.tradeSz.tradePrice == state.upperLimitPrice10000) {
                state.upRemainQty100 -= tick.tradeSz.tradeQty;

                if (state.upRemainQty100 < 0) {
                    uint32_t timestamp = static_cast<uint32_t>(
                        tick.tradeSz.transactTime - g_szOffsetTransactTime);
                    if (timestamp >= 9'30'00'000 && timestamp < 14'57'00'000) [[likely]] {
                        auto intent = checkWantBuyAtTimestamp(id, timestamp);
                        if (intent == WantBuy || ALWAYS_BUY) [[likely]] {
                            sendBuyRequest(id);
                        }


                        logStop(id, timestamp, intent);
                        return;
                    }
                }
            }
        }
    }
#endif

    pushTickToRing(id, tick);
}

void MDD::handleSnap(MDS::Snap &snap)
{
    if (snap.timestamp >= 9'25'00'000 && snap.timestamp < 9'26'00'000) {
        if (size_t index = arrayContains({g_prevLimitUpStockCodes, g_numPrevLimitUpStocks}, snap.stock); index != -1) {
            double openRate = static_cast<double>(snap.openPrice) / snap.preClosePrice - 1;
            g_prevLimitUpReturns[index] = openRate;
            g_prevLimitUpMeanReturn = computeMean({g_prevLimitUpReturns, g_numPrevLimitUpStocks});
            std::atomic_thread_fence(std::memory_order_seq_cst);
            SPDLOG_TRACE("prev-limit-up: stock={:06d} preClosePrice={} openPrice={} openRate={:.03f}% meanOpenRate={:.03f}%", stock, snap.preClosePrice, snap.openPrice, openRate * 100, g_prevLimitUpMeanReturn * 100);
        }
    }

// #if SELL_GC001
// #if SH
//     constexpr int32_t kGCStockCode = 204001; // GC001
// #endif
// #if SZ
//     constexpr int32_t kGCStockCode = 131810; // R-001
// #endif
//     if (stock == kGCStockCode && timestamp >= 14'57'00'000) {
//         static uint32_t gcState = 0;
//         if (gcState == 0) {
//             std::jthread(OES::queryAccountStatus).detach();
//         } else if (gcState == 5) {
//             OES::ReqOrder gcSellRequest;
//             std::atomic_thread_fence(std::memory_order_seq_cst);
//             int64_t quantity = (g_availableFund / 1000'00) * 10;
//             if (quantity > 100'0000) {
//                 quantity = 100'0000;
//             }
//             if (quantity > 80 || BYPASS_OES) {
//                 double price = snapPrice(snap, 1);
//                 makeGCSellRequest(gcSellRequest, stock, price, quantity);
//                 SPDLOG_DEBUG("sold GC001/R-001: stock={} price={} quantity={}", stock, price, quantity);
//                 OES::sendReqOrder(gcSellRequest);
//             }
//         }
//         ++gcState;
//     }
// #endif
}

void MDD::handleStatic(MDS::Stat &stat)
{
    int32_t id = linearizeStockId(stat.stock);
    if (id == -1) [[unlikely]] {
        return;
    }
    auto &state = g_stockStates[id];

    if (stat.upperLimitPrice == 0) [[unlikely]] {
        SPDLOG_WARN("invalid static price: stock={:06d} upperLimitPrice={}", stat.stock, stat.upperLimitPrice);
        unsubscribeStock(id);
        return;
    }


#if SH
    state.upperLimitPrice1000 = stat.upperLimitPrice * 10;
#endif
#if SZ
    state.upperLimitPrice10000 = stat.upperLimitPrice * 100;
#endif

    auto &compute = g_stockComputes[id];
    compute.stockCode = stat.stock;
    compute.upperLimitPrice = stat.upperLimitPrice;
    compute.preClosePrice = stat.preClosePrice;

    compute.openPrice = stat.preClosePrice;
    compute.upperLimitPriceApproach = static_cast<int32_t>(std::floor(stat.upperLimitPrice / 1.02)) - 1;
    // compute.upperLimitPriceApproach = static_cast<int32_t>(std::floor(stat.upperLimitPrice / 1.01)) - 1;
    // compute.upperLimitPriceApproach = stat.upperLimitPrice - 5;

    compute.fState.currSnapshot.lastPrice = stat.preClosePrice;
    compute.fState.currSnapshot.numTrades = 0;
    compute.fState.currSnapshot.volume = 0;
    compute.fState.currSnapshot.amount = 0;

    compute.fState.iirState = std::make_unique<IIRState>();
    compute.bState.iirState = std::make_unique<IIRState>();

    compute.fState.nextTickTimestamp = 9'30'00'000;

#if RECORD_FACTORS
    compute.factorListCache = std::make_unique<FactorList[]>(kWantSignLookAhead);
    memset(compute.factorListCache.get(), -1, sizeof(FactorList) * kWantSignLookAhead);
#endif

#if DUMMY_QUANTITY
    int32_t reportQuantity = 100;
#else
    int32_t reportQuantity = std::max(static_cast<int32_t>(std::min(std::round(
        (static_cast<double>(kReportMoney) / static_cast<double>(stat.upperLimitPrice))
        * 0.01), 9999.0)) * INT32_C(100), INT32_C(100));
#endif

    auto &buyRequest = g_buyRequests[id];
    makeBuyRequest(buyRequest, stat.stock, stat.upperLimitPrice, reportQuantity);
}

namespace
{

HEAT_ZONE_ORDBOOK void addComputeOrder(Compute &compute, MDS::Tick &tick)
{
#if SH
    if (tick.tickMergeSse.tickBSFlag == '1'
        && tick.tickMergeSse.price >= compute.upperLimitPriceApproach * 10) {
        Compute::UpSell sell;
        sell.price = tick.tickMergeSse.price / 10;
        sell.quantity = tick.tickMergeSse.qty / 1000;
        compute.upSellOrders.insert({tick.tickMergeSse.sellOrderNo, sell});
    }
#endif

#if SZ
    if (tick.orderSz.side == '2' && tick.orderSz.orderType == '2'
        && tick.orderSz.price >= compute.upperLimitPriceApproach * 100) {
        Compute::UpSell sell;
        sell.price = tick.orderSz.price / 100;
        sell.quantity = tick.orderSz.qty / 100;
        compute.upSellOrders.insert({tick.orderSz.applSeqNum, sell});
    }
#endif
}

HEAT_ZONE_ORDBOOK void addComputeCancel(Compute &compute, MDS::Tick &tick)
{
#if SH
    if (tick.tickMergeSse.tickBSFlag == '1') {
        auto it = compute.upSellOrders.find(tick.tickMergeSse.sellOrderNo);
        if (it != compute.upSellOrders.end()) {
            compute.upSellOrders.erase(it);
        }
    }
#endif

#if SZ
    if (tick.orderSz.price >= compute.upperLimitPriceApproach * 100) {
        Compute::UpSell sell;
        sell.price = tick.orderSz.price / 100;
        sell.quantity = tick.orderSz.qty / 100;
        compute.upSellOrders.insert({tick.orderSz.applSeqNum, sell});
    }
#endif
}

HEAT_ZONE_ORDBOOK void addRealTrade(Compute &compute, uint32_t timestamp, int32_t price, int32_t quantity)
{
    stepSnapshotUntil(compute, timestamp);

    int64_t q64 = quantity;
    compute.fState.currSnapshot.lastPrice = price;
    ++compute.fState.currSnapshot.numTrades;
    compute.fState.currSnapshot.volume += q64;
    compute.fState.currSnapshot.amount += price * q64;
}

HEAT_ZONE_ORDBOOK void addComputeTrade(Compute &compute, MDS::Tick &tick)
{
#if SH
    int32_t quantity = tick.tickMergeSse.qty / 1000;
    int32_t price = tick.tickMergeSse.price / 10;

    auto it = compute.upSellOrders.find(tick.tickMergeSse.sellOrderNo);
    if (it != compute.upSellOrders.end()) {
        it->second.quantity -= quantity;
        if (it->second.quantity <= 0) {
            compute.upSellOrders.erase(it);
        }
    }

    if (tick.tickMergeSse.tickTime < 9'30'00'00) {
        compute.fState.currSnapshot.lastPrice = compute.openPrice = price;
        compute.openVolume += quantity;
        return;
    }

    if (price >= compute.upperLimitPriceApproach) {
        compute.approachingLimitUp = true;
    }
    addRealTrade(compute, tick.tickMergeSse.tickTime * 10, price, quantity);
#endif

#if SZ
    int32_t quantity = tick.tradeSz.tradeQty / 100;
    int32_t price = tick.tradeSz.tradePrice / 100;

    auto it = compute.upSellOrders.find(tick.tradeSz.offerapplSeqnum);
    if (it != compute.upSellOrders.end()) {
        it->second.quantity -= quantity;
        if (it->second.quantity <= 0) {
            compute.upSellOrders.erase(it);
        }
    }

    uint32_t timestamp = static_cast<uint32_t>(tick.tradeSz.transactTime % UINT64_C(1'00'00'00'000));
    if (timestamp < 9'30'00'000) {
        compute.fState.currSnapshot.lastPrice = compute.openPrice = price;
        compute.openVolume += quantity;
        return;
    }

    if (price >= compute.upperLimitPriceApproach) {
        compute.approachingLimitUp = true;
    }
    addRealTrade(compute, timestamp, price, quantity);
#endif
}

COLD_ZONE void logLimitUp(int32_t id, uint32_t timestamp, Intent oldIntent)
{
    uint32_t linearTimestamp = (static_cast<uint32_t>(timestampLinear(timestamp)) + 90U) / 100U;
    uint64_t wantSign = g_wantSigns[id].load(std::memory_order_relaxed);
    uint32_t offset = linearTimestamp - static_cast<uint32_t>(wantSign);
    Intent nowIntent;
    if (offset >= kWantSignLookAhead) {
        nowIntent = NotSure;
    } else {
        nowIntent = static_cast<Intent>(static_cast<uint8_t>(wantSign >> (32 + offset * 2)) & 0b11);
    }

    auto &compute = g_stockComputes[id];
    auto &state = g_stockStates[id];

#if RECORD_FACTORS
    if (nowIntent != NotSure) {
        compute.factorListCache[offset].dumpFactors(timestampDelinear(linearTimestamp * 100), compute.stockCode);
    }
#endif
    SPDLOG_INFO("limit up model status: stock={:06d} price={} timestamp={} wantTime={} offset={} nowIntent={} oldIntent={}", compute.stockCode, compute.upperLimitPrice, timestamp, timestampDelinear(static_cast<uint32_t>(wantSign) * 100U), offset, nowIntent, oldIntent);
}

HEAT_ZONE_ORDBOOK void addComputeTick(Compute &compute, MDS::Tick &tick)
{
    if (compute.upperLimitPrice == 0) [[unlikely]] {
        return;
    }

#if SH
    if (tick.tickMergeSse.tickType == 0) [[unlikely]] {
        logLimitUp(&compute - g_stockComputes,
                   tick.tickMergeSse.tickTime * 10,
                   static_cast<Intent>(tick.tickMergeSse.tickBSFlag));
        compute.upperLimitPrice = 0;
        return;
    }

    switch (tick.tickMergeSse.tickType) {
    case 'A':
        addComputeOrder(compute, tick);
        break;
    case 'D':
        addComputeCancel(compute, tick);
        break;
    case 'T':
        addComputeTrade(compute, tick);
        break;
    default:
        break;
    }
#endif

#if SZ
    if (tick.messageType == 0) [[unlikely]] {
        logLimitUp(&compute - g_stockComputes,
                   tick.tradeSz.transactTime - g_szOffsetTransactTime,
                   static_cast<Intent>(tick.tradeSz.execType));
        compute.upperLimitPrice = 0;
        return;
    }

    if (tick.messageType == XeleCompat::MSG_TYPE_TRADE_SZ) {
        if (tick.tradeSz.execType == 0x1) {
            addComputeCancel(compute, tick);
        } else {
            addComputeTrade(compute, tick);
        }
    } else if (tick.messageType == XeleCompat::MSG_TYPE_ORDER_SZ) [[likely]] {
        addComputeOrder(compute, tick);
    }
#endif
}

HEAT_ZONE_TIMER void computeThreadMain(int32_t channel, std::stop_token stop)
{
    auto &tickRing = g_tickRings[channel];

    alignas(4096) thread_local MDS::Tick tickBuf[1024];
    std::vector<int32_t> approachStockIds;
    std::vector<int64_t> remainSellAmounts;
    approachStockIds.reserve(8);
    remainSellAmounts.reserve(8);

    const int32_t startId = (g_numStocks * channel) / kChannelCount;
    const int32_t stopId = (g_numStocks * (channel + 1)) / kChannelCount;

    while (!stop.stop_requested()) [[likely]] {
        approachStockIds.clear();
        for (int32_t id = startId; id != stopId; ++id) {
            g_stockComputes[id].approachingLimitUp = false;
        }

        while (true) {
            MDS::Tick *endTicks = g_tickRings[channel].read_some(tickBuf, tickBuf + std::size(tickBuf));
            for (MDS::Tick *pTick = tickBuf; pTick != endTicks; ++pTick) {
                int32_t stock = securityId(pTick->commonHead.securityID);
                int32_t id = linearizeStockId(stock);
                if (id == -1) [[unlikely]] {
                    continue;
                }
                if (id < startId || id >= stopId) [[unlikely]] {
                    throw std::out_of_range("received tick id out of range");
                }
                addComputeTick(g_stockComputes[id], *pTick);
            }
            if (endTicks < tickBuf + std::size(tickBuf) / 2) {
                break;
            }
        }

        for (int32_t id = startId; id != stopId; ++id) {
            auto &compute = g_stockComputes[id];
            if (compute.approachingLimitUp) {
                approachStockIds.push_back(id);
            }
        }

        computeFutureWantBuy(approachStockIds);
    }
}

}

void MDD::start(const char *config)
{
    parseDailyConfig(config);
    SPDLOG_INFO("found {} stocks", g_numStocks);

    monotonicSleepFor(5'000'000);

#if !BYPASS_OES
    SPDLOG_INFO("initializing trade api");
    OES::start(config);
    monotonicSleepFor(5'000'000);
#endif

    SPDLOG_INFO("initializing quote api");
    MDS::start(config);

#if !BYPASS_OES
    while (!OES::isStarted()) {
        SPDLOG_DEBUG("wait for trade initial");
        monotonicSleepFor(400'000'000);
    }

    OES::queryAccountStatus();
    monotonicSleepFor(200'000'000);
#endif

    SPDLOG_INFO("subscribing {} stocks", g_numStocks);
    MDS::subscribe(g_stockCodes, g_numStocks);

    monotonicSleepFor(10'000'000);
    SPDLOG_DEBUG("starting {} compute threads", kChannelCount);
    for (int32_t c = 0; c < kChannelCount; ++c) {
        g_computeThreads[c] = std::jthread([c] (std::stop_token stop) {
            setThisThreadAffinity(kChannelCpus[c]);
            SPDLOG_TRACE("compute thread started: channel={}", c);
            computeThreadMain(c, stop);
        });
    }
    monotonicSleepFor(120'000'000);

    SPDLOG_INFO("start receiving stock quotes");
    MDS::startReceive();
    while (!MDS::isStarted()) {
        monotonicSleepFor(10'000'000);
    }
    SPDLOG_DEBUG("mds receive started");
    monotonicSleepFor(10'000'000);

    SPDLOG_INFO("system fully started");
}

void MDD::stop()
{
    monotonicSleepFor(20'000'000);
    SPDLOG_DEBUG("stopping mds");
    MDS::stop();
    monotonicSleepFor(30'000'000);

#if !BYPASS_OES
    SPDLOG_DEBUG("stopping oes");
    OES::stop();
    monotonicSleepFor(60'000'000);
#endif

    SPDLOG_DEBUG("stopping {} compute threads", kChannelCount);
    for (int32_t c = 0; c < kChannelCount; ++c) {
        g_computeThreads[c].request_stop();
        g_computeThreads[c].join();
    }

    SPDLOG_INFO("system fully stopped");
}

void MDD::requestStop()
{
    MDS::requestStop();
}

bool MDD::isFinished()
{
    return MDS::isFinished();
}

HEAT_ZONE_RSPORDER void MDD::handleRspOrder(OES::RspOrder &rspOrder)
{
#if XC || NE
    std::string body;
    switch (rspOrder.rspType) {
#define PER_TYPE(name) case OES::RspOrder::Xele##name: body = refl::to_string(*rspOrder.xele##name); break;
        PER_TYPE(RspOrderInsert)
        PER_TYPE(RspOrderAction)
        PER_TYPE(RtnOrder)
        PER_TYPE(RtnTrade)
#undef PER_TYPE
        default: body = "?"; break;
    }
    SPDLOG_DEBUG("order response: rspType={} requestID={} errorID={} errorMsg=`{}` userLocalID={} | {}",
                rspOrder.rspType,
                rspOrder.requestID,
                rspOrder.errorID,
                strXeleError(rspOrder.errorID),
                rspOrder.userLocalID,
                body);

#elif OST
    std::string body;
    switch (rspOrder.rspType) {
#define PER_TYPE(name) case OES::RspOrder::Ost##name: body = refl::to_string(*rspOrder.ost##name); break;
        PER_TYPE(RspOrderInsert)
        PER_TYPE(RspOrderAction)
        PER_TYPE(RtnOrder)
        PER_TYPE(ErrRtnOrderAction)
        PER_TYPE(RtnTrade)
#undef PER_TYPE
        default: body = "?"; break;
    }
    SPDLOG_DEBUG("order response: rspType={} requestID={} errorID={} errorMsg=`{}` userLocalID={} | {}",
                rspOrder.rspType,
                rspOrder.requestID,
                rspOrder.errorID,
                rspOrder.errorMsg,
                rspOrder.userLocalID,
                body);
#endif

// #if REPLAY
//     int32_t stock = rspOrder.stockCode;
// #elif XC || NE
//     int32_t stock = rspOrder.userLocalID;
// #elif OST
//     int32_t stock = rspOrder.userLocalID;
// #endif
//     int32_t id = linearizeStockId(stock);
//     if (id == -1) [[unlikely]] {
//         return;
//     }
}

void MDD::handleAccountFund(OES::AccountFund &fund)
{
    SPDLOG_INFO("fund status: availableFund={} initTotalFund={}", fund.availableFund, fund.initTotalFund);
    g_availableFund = fund.availableFund;
    g_initTotalFund = fund.initTotalFund;
}

void MDD::handleAccountPosition(OES::AccountPosition *positions, size_t nPosition)
{
    SPDLOG_INFO("account position status: nPosition={}", nPosition);
    for (size_t i = 0; i < nPosition; ++i) {
        SPDLOG_INFO("position #{}: stock={:06d} totalPos={} availPos={}", i, positions[i].stock, positions[i].totalPosition, positions[i].availablePosition);
    }
}
