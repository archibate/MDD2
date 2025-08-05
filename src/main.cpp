#include <spdlog/spdlog.h>
#include "MDD.h"
#include "config.h"
#include <stdexcept>
#include <thread>
#include <chrono>
#include <signal.h>
#include <string.h>

namespace
{

void signalHandler(int signo)
{
    if (signo == SIGINT) {
        ::signal(SIGINT, SIG_DFL);
        SPDLOG_WARN("received SIGINT, stopping gracefully");
        MDD::requestStop();

    } else if (signo == SIGHUP) {
        SPDLOG_WARN("received SIGHUP, ignorning");
    }
}

void segvHandler(int sig, siginfo_t *info, void *ucontext) {
    signal(SIGSEGV, SIG_DFL);
    std::fprintf(stderr, "Caught SIGSEGV at address: %p\n", info->si_addr);
    std::fflush(stderr);
    SPDLOG_ERROR("Caught SIGSEGV at address: {}", info->si_addr);
    std::_Exit(EXIT_FAILURE);
}

}

int main(int argc, char **argv)
{
    SPDLOG_CRITICAL("program started, compiled at {} {}", __DATE__, __TIME__);
#define COMPILE_FLAG(x) SPDLOG_INFO("compile flag: " #x "={}", x);
    COMPILE_FLAG(BUILD_SPEED)
    COMPILE_FLAG(RECORD_FACTORS)
    COMPILE_FLAG(ALWAYS_BUY)
    COMPILE_FLAG(ASYNC_LOGGER)
    COMPILE_FLAG(TARGET_SECURITY)
    COMPILE_FLAG(TARGET_MARKET)
#undef COMPILE_FLAG
    for (int i = 0; i < argc; ++i) {
        SPDLOG_INFO("runtime argument: argv[{}]=`{}`", i, argv[i]);
    }

    SPDLOG_CRITICAL("now starting MDD v2 trading system");
    MDD::start(argv[1] ?: "config.json");

    SPDLOG_DEBUG("registering signal handlers");
    signal(SIGINT, signalHandler);
    signal(SIGHUP, signalHandler);
    struct sigaction sa;
    sa.sa_flags = SA_SIGINFO;
    sa.sa_sigaction = segvHandler;
    sigemptyset(&sa.sa_mask);
    if (sigaction(SIGSEGV, &sa, NULL) == -1) {
        SPDLOG_WARN("failed to hook SIGSEGV");
    }

    SPDLOG_CRITICAL("MDD v2 started, now waiting for finish");
    while (!MDD::isFinished()) {
        std::this_thread::sleep_for(std::chrono::seconds(1));
    }

    SPDLOG_CRITICAL("MDD v2 system stopping");
    signal(SIGINT, SIG_DFL);
    signal(SIGHUP, SIG_DFL);
    signal(SIGSEGV, SIG_DFL);
    MDD::stop();

    SPDLOG_CRITICAL("program exiting");
    return 0;
}
