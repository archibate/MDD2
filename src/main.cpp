#include <spdlog/spdlog.h>
#include "MDD.h"
#include <thread>
#include <chrono>


int main()
{
    MDD::start();

    while (!MDD::isFinished()) {
        std::this_thread::sleep_for(std::chrono::seconds(1));
    }

    MDD::stop();
    return 0;
}
