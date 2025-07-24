#include <cstdio>
#include <spdlog/spdlog.h>
#include <spdlog/async_logger.h>
#include <spdlog/sinks/basic_file_sink.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/details/thread_pool.h>

namespace
{
struct LoggerInit
{
    LoggerInit() {
        std::remove("MDD.log");
        static std::vector<spdlog::sink_ptr> sinks = {
            std::make_shared<spdlog::sinks::stdout_color_sink_mt>(),
            std::make_shared<spdlog::sinks::basic_file_sink_mt>("MDD.log"),
        };
        sinks[0]->set_level(spdlog::level::debug);
        sinks[1]->set_level(spdlog::level::trace);
        // static auto pool = std::make_shared<spdlog::details::thread_pool>(2048, 1);
        // static auto logger = std::make_shared<spdlog::async_logger>(
        //     "MDD", sinks.begin(), sinks.end(), pool);
        static auto logger = std::make_shared<spdlog::logger>(
            "MDD", sinks.begin(), sinks.end());
        logger->set_level(spdlog::level::trace);
        logger->flush_on(spdlog::level::debug);
        spdlog::set_default_logger(logger);
        spdlog::set_level(spdlog::level::trace);
    }
};

LoggerInit loggerInit;
}
