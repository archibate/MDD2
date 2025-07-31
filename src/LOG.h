#pragma once


#include <sstream>
#include <cstdio>
#include <cstdarg>


#if !__has_include(<spdlog/spdlog.h>)
#include <iostream>

class LogClass
{
    std::ostringstream ss;

public:
    LogClass(LogClass &&) = delete;

    explicit LogClass(const char *level)
    {
        ss << '[' << level << ']';
    }

    template <class T>
    LogClass &operator<<(T const &t)
    {
        ss << ' ' << t;
        return *this;
    }

    ~LogClass() noexcept(false)
    {
        ss << '\n';
        std::cout << ss.str();
    }

    LogClass &operator()(const char *fmt, ...) {
        std::va_list va;
        va_start(va, fmt);
        int n = std::vsnprintf(nullptr, 0, fmt, va);
        if (n > 0) [[likely]] {
            va_end(va);
            va_start(va, fmt);
            std::string s(n, '\0');
            int n = std::vsnprintf(s.data(), n, fmt, va);
            if (n > 0) [[likely]] {
                s.resize(n);
                if (s.back() == '\n') {
                    s.pop_back();
                }
                this->operator<<(s);
            }
        }
        va_end(va);
        return *this;
    }
};

#define LOG(level) LogClass(#level)
#define LOGf(level, ...) LOG(level)(__VA_ARGS__)

#else
#include <spdlog/spdlog.h>

class LogClass
{
    std::ostringstream ss;
    spdlog::level::level_enum level;
    spdlog::source_loc loc;

public:
    LogClass(LogClass &&) = delete;

    explicit LogClass(spdlog::level::level_enum level, spdlog::source_loc loc)
        : level(level), loc(loc)
    {
        ss << "(ZHAO)";
    }

    template <class T>
    LogClass &operator<<(T const &t)
    {
        ss << ' ' << t;
        return *this;
    }

    ~LogClass() noexcept(false)
    {
        auto s = ss.str();
        while (!s.empty() && s.back() == '\n') {
            s.pop_back();
        }
        spdlog::default_logger_raw()->log(loc, level, s);
    }

    LogClass &operator()(const char *fmt, ...) {
        std::va_list va;
        va_start(va, fmt);
        int n = std::vsnprintf(nullptr, 0, fmt, va);
        va_end(va);
        if (n > 0) [[likely]] {
            std::string s(n, '*');
            va_start(va, fmt);
            int n = std::vsprintf(s.data(), fmt, va);
            va_end(va);
            if (n > 0) [[likely]] {
                s.resize(n);
                ss << ' ' << s;
            }
        }
        return *this;
    }

    class Levels {
        Levels() = default;

    public:
        constexpr static auto TRACE = spdlog::level::trace;
        constexpr static auto DEBUG = spdlog::level::debug;
        constexpr static auto INFO = spdlog::level::info;
        constexpr static auto WARN = spdlog::level::warn;
        constexpr static auto ERROR = spdlog::level::err;
        constexpr static auto CRIT = spdlog::level::critical;
    };
};


#define LOG(level) LogClass(LogClass::Levels::level, spdlog::source_loc{__FILE__, __LINE__, SPDLOG_FUNCTION})
#define LOGf(level, ...) LOG(level)(__VA_ARGS__)
#endif
