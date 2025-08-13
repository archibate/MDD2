#include <spdlog/spdlog.h>
#include "MDD.h"
#include "config.h"
#include "dateTime.h"
#include <stdexcept>
#include <algorithm>
#include <thread>
#include <chrono>
#include <cstring>
#include <cstdio>
#include <fstream>
#include <signal.h>
#include <unistd.h>
#include <execinfo.h>

namespace
{

void signalHandler(int signo)
{
    if (signo == SIGINT) {
        signal(SIGINT, SIG_DFL);
        SPDLOG_WARN("received SIGINT, stopping gracefully");
        MDD::requestStop();

    } else if (signo == SIGHUP) {
        SPDLOG_WARN("received SIGHUP, ignorning");
    }
}

void abortHandler(int signo)
{
    signal(signo, SIG_DFL);

    void *addrs[32];
    size_t size = backtrace(addrs, std::size(addrs));
    backtrace_symbols_fd(addrs, size, STDERR_FILENO);
    char **symbols = backtrace_symbols(addrs, size);
    SPDLOG_ERROR("Stack backtrace:");
    for (size_t i = 0; i < size; i++) {
        SPDLOG_ERROR("  {}", symbols[i]);
    }
    SPDLOG_ERROR("PROGRAM ABORTED");
    auto ep = std::current_exception();
    if (!ep) {
        SPDLOG_ERROR("aborted with no exception");
    } else {
        try {
            std::rethrow_exception(ep);
        } catch (std::exception const &e) {
            SPDLOG_ERROR("`{}` exception thrown: {}", typeid(e).name(), e.what());
        } catch (...) {
            SPDLOG_ERROR("unknown exception thrown");
        }
    }

    SPDLOG_CRITICAL("faulty exiting with {}", signo);
    _Exit(signo);
}

static pid_t mainThreadId = gettid();

void faultHandler(int signo, siginfo_t *info, void *ucontext) {
    signal(signo, SIG_DFL);

    fprintf(stderr, "%s at address: %p\n", strsignal(signo), info->si_addr);
    fflush(stderr);

    void *addrs[32];
    size_t size = backtrace(addrs, std::size(addrs));
    backtrace_symbols_fd(addrs, size, STDERR_FILENO);
    SPDLOG_ERROR("PROGRAM CRASHED");
    char **symbols = backtrace_symbols(addrs, size);
    SPDLOG_ERROR("Stack backtrace:");
    for (size_t i = 0; i < size; i++) {
        SPDLOG_ERROR("  {}", symbols[i]);
    }

    SPDLOG_ERROR("Caught signal: {} ({})", signo, strsignal(signo));
    SPDLOG_ERROR("Fault address: {}", info->si_addr);
    SPDLOG_ERROR("Thread ID: {}", gettid());
    SPDLOG_ERROR("Main thread ID: {}", mainThreadId);

    if (signo == SIGSEGV) {
        static const char *const kSegvCodeNames[] = {
            "SEGV_MAPERR",
            "SEGV_ACCERR",
            "SEGV_BNDERR",
            "SEGV_PKUERR",
            "SEGV_ACCADI",
            "SEGV_ADIDERR",
            "SEGV_ADIPERR",
            "SEGV_MTEAERR",
            "SEGV_MTESERR",
        };
        SPDLOG_ERROR("Signal code: {} ({})", info->si_code, info->si_code > 0
                     && info->si_code <= std::size(kSegvCodeNames)
                     ? kSegvCodeNames[info->si_code - 1] : "???");
    } else {
        SPDLOG_ERROR("Signal code: {}", info->si_code);
    }
    SPDLOG_ERROR("Signal errno: {} ({})", info->si_errno, strerror(info->si_errno));

    std::ifstream fin("/proc/self/maps");
    if (fin.is_open()) {
        std::string line;
        SPDLOG_ERROR("Memory map:");
        while (std::getline(fin, line)) {
            SPDLOG_ERROR("  {}", line);
        }
        fin.close();
    }

    SPDLOG_CRITICAL("faulty exiting with {}", signo);
    _Exit(signo);
}

}

int main(int argc, char **argv)
{
    SPDLOG_CRITICAL("program started, compiled at {} {}", __DATE__, __TIME__);
#define COMPILE_FLAG(x) SPDLOG_DEBUG("compile flag: " #x "={}", x);
    COMPILE_FLAG(BUILD_SPEED)
    COMPILE_FLAG(RECORD_FACTORS)
    COMPILE_FLAG(LIMIT_SPEED)
    COMPILE_FLAG(ALWAYS_BUY)
    COMPILE_FLAG(DUMMY_QUANTITY)
    COMPILE_FLAG(ASYNC_LOGGER)
    COMPILE_FLAG(BEST_ORDER)
    COMPILE_FLAG(SPLIT_ORDER)
    COMPILE_FLAG(SZ_IS_SECOND)
    COMPILE_FLAG(TARGET_SECURITY)
    COMPILE_FLAG(TARGET_MARKET)
#undef COMPILE_FLAG
    for (int i = 0; i < argc; ++i) {
        SPDLOG_DEBUG("runtime argument: argv[{}]=`{}`", i, argv[i]);
    }

    SPDLOG_CRITICAL("now starting MDD v2 trading system");
    MDD::start(argv[1] ?: "config.json");

    SPDLOG_DEBUG("hooking signal handlers");
    signal(SIGINT, signalHandler);
    signal(SIGHUP, signalHandler);
    signal(SIGABRT, abortHandler);
    struct sigaction sa;
    sa.sa_flags = SA_SIGINFO;
    sa.sa_sigaction = faultHandler;
    sigemptyset(&sa.sa_mask);
    sigaction(SIGSEGV, &sa, nullptr);
    sigaction(SIGILL, &sa, nullptr);
    sigaction(SIGBUS, &sa, nullptr);
    sigaction(SIGFPE, &sa, nullptr);

    SPDLOG_CRITICAL("MDD v2 started, now waiting for finish");
    while (!MDD::isFinished()) {
        std::this_thread::sleep_for(std::chrono::seconds(1));
#if !REPLAY
        int32_t now = getTimestamp();
        static uint32_t counter = 0;
        if (counter++ % 60 == 0) {
            SPDLOG_TRACE("now time: {}", now);
        }
        if (now >= 15'00'03'000) {
            SPDLOG_INFO("MDD v2 exiting at market close time {}", now);
            break;
        }
#endif
    }

    SPDLOG_CRITICAL("MDD v2 system stopping");
    signal(SIGINT, SIG_DFL);
    signal(SIGHUP, SIG_DFL);
    signal(SIGABRT, SIG_DFL);
    signal(SIGSEGV, SIG_DFL);
    signal(SIGILL, SIG_DFL);
    signal(SIGBUS, SIG_DFL);
    signal(SIGFPE, SIG_DFL);
    SPDLOG_DEBUG("de-hooked signal handlers");
    MDD::stop();

    SPDLOG_CRITICAL("program exiting");
    return 0;
}
