#include <cstdio>
#include <spdlog/spdlog.h>
#if ASYNC_LOGGER
#include <spdlog/async_logger.h>
#include <spdlog/details/thread_pool.h>
#else
#include <spdlog/logger.h>
#endif
#include <spdlog/sinks/basic_file_sink.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include "config.h"
#include "dateTime.h"

namespace
{
struct LoggerInit
{
    LoggerInit() {
#if REPLAY
        const char *logFile = "MDD.log";
        std::remove(logFile);
#else
        std::string logFile = "MDD-" + std::to_string(getToday()) + ".log";
#endif
        static std::vector<spdlog::sink_ptr> sinks = {
            std::make_shared<spdlog::sinks::stdout_color_sink_mt>(),
            std::make_shared<spdlog::sinks::basic_file_sink_mt>(logFile),
        };
        sinks[0]->set_level(spdlog::level::debug);
        sinks[1]->set_level(spdlog::level::trace);
#if ASYNC_LOGGER
        static auto pool = std::make_shared<spdlog::details::thread_pool>(2048, 1);
        static auto logger = std::make_shared<spdlog::async_logger>(
            "MDD", sinks.begin(), sinks.end(), pool);
#else
        static auto logger = std::make_shared<spdlog::logger>(
            "MDD", sinks.begin(), sinks.end());
#endif
        logger->set_pattern("%C%m%d %H%M%S.%f %^%L (%s:%#) %v%$");
        logger->set_level(spdlog::level::trace);
        logger->flush_on(spdlog::level::warn);
        spdlog::set_default_logger(logger);
        spdlog::set_level(spdlog::level::trace);
    }
};

LoggerInit loggerInit;
}
