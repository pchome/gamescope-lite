#pragma once
#include <cstdarg>
#include <cstdint>

#include <format>
#include <functional>
#include <memory>
#include <print>
#include <string_view>

#include "GamescopeVersion.h"

#ifdef __GNUC__
#define ATTRIB_PRINTF(start, end) __attribute__((format(printf, start, end)))
#else
#define ATTRIB_PRINTF(start, end)
#endif

enum LogPriority
{
	LOG_SILENT,
	LOG_ERROR,
	LOG_WARNING,
	LOG_INFO,
	LOG_DEBUG,
};

struct LogConVar_t;

using std::operator""sv;

class LogScope
{
public:
	LogScope( std::string_view psvName, LogPriority eMaxPriority = LOG_INFO );
	LogScope( std::string_view psvName, std::string_view psvPrefix, LogPriority eMaxPriority = LOG_INFO );
	~LogScope();

	bool Enabled( LogPriority ePriority ) const;
	void SetPriority( LogPriority ePriority ) { m_eMaxPriority = ePriority; }

	void vlogf(enum LogPriority priority, const char *fmt, va_list args) ATTRIB_PRINTF(3, 0);
	void log(enum LogPriority priority, std::string_view psvText);

	void warnf(const char *fmt, ...) ATTRIB_PRINTF(2, 3);
	void errorf(const char *fmt, ...) ATTRIB_PRINTF(2, 3);
	void infof(const char *fmt, ...) ATTRIB_PRINTF(2, 3);
	void debugf(const char *fmt, ...) ATTRIB_PRINTF(2, 3);

	void errorf_errno(const char *fmt, ...) ATTRIB_PRINTF(2, 3);

    constexpr auto prefix(std::string const& fmt) {
      return std::format("[{}]{}[\e[1;30m{}\e[0m]: ", gamescope::build::project_name, fmt, m_psvPrefix);
    }
    constexpr auto eprefix() { return prefix("[\e[0;31m" "Error" "\e[0m]"); }
    constexpr auto wprefix() { return prefix("[\e[0;33m" "Warn " "\e[0m]"); }
    constexpr auto iprefix() { return prefix("[\e[0;34m" "Info " "\e[0m]"); }
    constexpr auto dprefix() { return prefix("[\e[0;35m" "Debug" "\e[0m]"); }

    template <typename... Args>
    constexpr auto msg(std::format_string<Args...> const fmt, Args&&... args) {
        return std::format(fmt, std::forward<Args>(args)...);
    }
    template <typename... Args>
    constexpr void error(std::format_string<Args...> const fmt, Args&&... args) {
        auto pre = std::format("{}", eprefix());
        std::println(stderr, "{}{}", pre, msg(fmt, std::forward<Args>(args)...));
    }
    template <typename... Args>
    constexpr void warn(std::format_string<Args...> const fmt, Args&&... args) {
        auto pre = std::format("{}", wprefix());
        std::println(stderr, "{}{}", pre, msg(fmt, std::forward<Args>(args)...));
    }
    template <typename... Args>
    constexpr void info(std::format_string<Args...> const fmt, Args&&... args) {
        auto pre = std::format("{}", iprefix());
        std::println(stderr, "{}{}", pre, msg(fmt, std::forward<Args>(args)...));
    }
    template <typename... Args>
    constexpr void debug(std::format_string<Args...> const fmt, Args&&... args) {
        auto pre = std::format("{}", dprefix());
        std::println(stderr, "{}{}", pre, msg(fmt, std::forward<Args>(args)...));
    }
    template <typename... Args>
    constexpr void log(std::format_string<Args...> const fmt, Args&&... args) {
        std::println(stdout, fmt, std::forward<Args>(args)...);
    }

	bool bPrefixEnabled = true;

	using LoggingListenerFunc = std::function<void( LogPriority ePriority, std::string_view psvScope, std::string_view psvText )>;
	std::unordered_map<uintptr_t, LoggingListenerFunc> m_LoggingListeners;

private:
	void vprintf(enum LogPriority priority, const char *fmt, va_list args) ATTRIB_PRINTF(3, 0);
	void logf(enum LogPriority priority, const char *fmt, ...) ATTRIB_PRINTF(3, 4);

	std::string_view m_psvName;
	std::string_view m_psvPrefix;

	LogPriority m_eMaxPriority = LOG_INFO;
	
	std::unique_ptr<LogConVar_t> m_pEnableConVar;
};
