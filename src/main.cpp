#include <spdlog/spdlog.h>
#include "MDD.h"
#include <thread>
#include <chrono>


int main(int argc, char **argv)
{
    MDD::start(argv[1] ?:
#if REPLAY
               "config_replay.json"
#else
               "config.json"
#endif
               );

    while (!MDD::isFinished()) {
        std::this_thread::sleep_for(std::chrono::seconds(1));
    }

    MDD::stop();
    return 0;
}
