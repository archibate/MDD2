#include "src/radixSort.h"
#include <algorithm>
#include <random>
#include <iostream>
#include <vector>


int main()
{
    std::vector<std::pair<uint32_t, uint32_t>> a;
    std::generate_n(std::back_inserter(a), 100, [rng = std::mt19937{}, uni = std::uniform_int_distribution<uint32_t>{0, 10000}] () mutable {
        return std::make_pair(uni(rng), uni(rng));
    });
    radixSort<8, 4, sizeof(uint32_t), 0, sizeof(decltype(a)::value_type)>(a.data(), a.size());
    for (auto x: a) {
        std::cout << x.first << ' ' << x.second << '\n';
    }
}
