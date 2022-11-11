#include <logger/logger.h>
#include <logger/platform_config.h>
#include <logger/asserts.h>
#include <logger/time_helper.h>

#if defined(SERVER_LIB_PLATFORM_LINUX)
#include <pthread.h>
#include <sys/syscall.h> //SYS_gettid
#include <sys/syscall.h>
#include <syslog.h>
#include <unistd.h>
#elif defined(SERVER_LIB_PLATFORM_WINDOWS)
#include <windows.h>
#endif //! SERVER_LIB_PLATFORM_LINUX

#include <iostream>
#include <iomanip>
#include <chrono>
#include <fstream>

#include "logging_trace.h"

namespace server_lib {
namespace {
    using thread_info_type = logger::log_context::thread_info_type;

    thread_info_type get_thread_info()
    {
        uint64_t thread_id = 0;
        std::string thread_name;
#if defined(SERVER_LIB_PLATFORM_LINUX)
        thread_id = static_cast<decltype(thread_id)>(syscall(SYS_gettid));
        static int MAX_THREAD_NAME_SZ = 15;
        char buff[MAX_THREAD_NAME_SZ + 1];
        if (!pthread_getname_np(pthread_self(), buff, sizeof(buff)))
        {
            thread_name = buff;
        }
#endif
        return std::make_tuple(thread_id, thread_name, false);
    }

    uint64_t get_thread_id(const thread_info_type& thread_info)
    {
        return std::get<0>(thread_info);
    }

    const std::string& get_thread_name(const thread_info_type& thread_info)
    {
        return std::get<1>(thread_info);
    }

    bool get_thread_main(const thread_info_type& thread_info)
    {
        return std::get<2>(thread_info);
    }

    void set_thread_main(thread_info_type& thread_info, bool main)
    {
        if (get_thread_main(thread_info) != main)
        {
            thread_info = std::make_tuple(get_thread_id(thread_info),
                                          get_thread_name(thread_info),
                                          main);
        }
    }

    static auto s_main_thread_info = get_thread_info();

    std::string get_application_name()
    {
        std::string name;
#if defined(SERVER_LIB_PLATFORM_LINUX)

        std::ifstream("/proc/self/comm") >> name;

        // TODO: trimmed!

#elif defined(SERVER_LIB_PLATFORM_WINDOWS)

        char buf[MAX_PATH];
        GetModuleFileNameA(nullptr, buf, MAX_PATH);
        name = buf;

#endif
        return name;
    }

    static auto s_this_application_name = get_application_name();
} // namespace

std::atomic_ulong logger::log_context::s_id_counter(0u);

const char* logger::default_time_format = "%Y-%m-%dT%H:%M:%S";

const int logger::level_trace = static_cast<int>(logger::level::fatal);
const int logger::level_debug = static_cast<int>(logger::level::trace);
const int logger::level_info = level_debug + static_cast<int>(logger::level::debug);
const int logger::level_warning = level_info + static_cast<int>(logger::level::info);
const int logger::level_error = level_warning + static_cast<int>(logger::level::warning);

const int logger::details_all = static_cast<int>(logger::details::all);
const int logger::details_without_app_name = static_cast<int>(logger::details::without_app_name);
// clang-format off
const int logger::details_message_with_level = static_cast<int>(logger::details::without_app_name) +
                                               static_cast<int>(logger::details::without_time) +
                                               static_cast<int>(logger::details::without_microseconds) +
                                               static_cast<int>(logger::details::without_thread_info) +
                                               static_cast<int>(logger::details::without_source_code);
const int logger::details_message_without_source_code = static_cast<int>(logger::details::without_app_name) +
                                               static_cast<int>(logger::details::without_source_code);
const int logger::details_message_only = details_message_with_level +
                                         static_cast<int>(logger::details::without_level);
// clang-format on

logger::log_context::log_context()
    : id(s_id_counter++)
{
    thread_info = get_thread_info();
    if (get_thread_id(thread_info) == get_thread_id(s_main_thread_info))
        set_thread_main(thread_info, true);
}

void logger::add_cli_destination()
{
    static bool added = false;
    if (added)
        return;

    auto to_cli_level = [](level lv) {
        switch (lv)
        {
        case logger::level::trace:
            return "[trace] ";
        case logger::level::debug:
            return "[debug] ";
        case logger::level::info:
            return "[info] ";
        case logger::level::warning:
            return "[warning] ";
        case logger::level::error:
            return "[error!] ";
        case logger::level::fatal:
            return "[fatal!!!] ";
        default:;
        }
        return "";
    };

    auto cli_write = [this, to_cli_level](const log_message& msg, int details_filter) {
        using std::chrono::system_clock;
        auto now = system_clock::now();

        std::lock_guard<std::mutex> lock(_mutex_for_row);

        if (~details_filter & static_cast<int>(logger::details::without_app_name))
        {
            std::cout << s_this_application_name << ": ";
        }

        if (~details_filter & static_cast<int>(logger::details::without_time))
        {
            std::cout << to_iso_string(system_clock::to_time_t(now), _time_format.c_str(), false);
            if (~details_filter & static_cast<int>(logger::details::without_microseconds))
            {
                auto transformed = now.time_since_epoch().count() / 1000;
                auto micro = transformed % 1000000;
                std::cout << '.' << micro << ' ';
            }
        }
        if (~details_filter & static_cast<int>(logger::details::without_level))
        {
            std::cout << std::setw(11) << to_cli_level(msg.context.lv);
        }
        if (~details_filter & static_cast<int>(logger::details::without_thread_info) && !get_thread_main(msg.context.thread_info))
        {
            std::cout << '[';
            std::cout << get_thread_id(msg.context.thread_info);
            const auto& name = get_thread_name(msg.context.thread_info);
            if (!name.empty())
            {
                std::cout << '-';
                std::cout << get_thread_name(msg.context.thread_info);
            }
            std::cout << ']';
        }

        std::cout << msg.message.str();

        if (~details_filter & static_cast<int>(logger::details::without_source_code))
        {
            std::cout << " (from " << msg.context.file << ':' << msg.context.line << ')';
        }
        std::cout << std::endl;
    };
    add_destination(std::move(cli_write));

    added = true;
}

void logger::add_syslog_destination()
{
    static bool added = false;
    if (added)
        return;

#if defined(SERVER_LIB_PLATFORM_LINUX)
    auto to_syslog_level = [](level lv) -> int {
        switch (lv)
        {
        case logger::level::trace:
            return LOG_DEBUG;
        case logger::level::debug:
            return LOG_DEBUG;
        case logger::level::info:
            return LOG_INFO;
        case logger::level::warning:
            return LOG_WARNING;
        case logger::level::error:
            return LOG_ERR;
        case logger::level::fatal:
            return LOG_CRIT;
        default:;
        }
        return LOG_DEBUG;
    };

    auto syslog_write = [to_syslog_level](const log_message& msg, int details_filter) {
        if (~details_filter & static_cast<int>(logger::details::without_source_code))
        {
            syslog(to_syslog_level(msg.context.lv), "%s (from %s:%d)",
                   msg.message.str().c_str(),
                   msg.context.file.c_str(), msg.context.line);
        }
        else
        {
            syslog(to_syslog_level(msg.context.lv), "%s",
                   msg.message.str().c_str());
        }
    };
    add_destination(std::move(syslog_write));
#else // SERVER_LIB_PLATFORM_LINUX
    SRV_ERROR("Not implemented");
#endif // !SERVER_LIB_PLATFORM_LINUX

    added = true;
}

logger::logger() { _logs_on = false; }

logger& logger::init_sys_log()
{
    // Use init_sys_log to switch on syslog instead obsolete macro
    // USE_NATIVE_SYSLOG
#if defined(SERVER_LIB_PLATFORM_LINUX)
    openlog(NULL, LOG_CONS | LOG_PID | LOG_NDELAY, LOG_LOCAL2);
#endif
    add_syslog_destination();

    unlock();
    return *this;
}

logger& logger::init_cli_log(const char* time_format)
{
    _time_format = time_format;
    add_cli_destination();

    unlock();
    return *this;
}

logger& logger::set_level(int filter)
{
    // clang-format off
    static const int max_filter = static_cast<int>(level::error) +
                                  static_cast<int>(level::warning) +
                                  static_cast<int>(level::info) +
                                  static_cast<int>(level::debug) +
                                  static_cast<int>(level::trace);
    // clang-format on
    SRV_ASSERT(filter >= 0 && filter <= max_filter);

    _level_filter = filter;
    return *this;
}

logger& logger::set_level_from_environment(const char* var_name)
{
    char* var_val = getenv(var_name);
    if (var_val != NULL)
    {
        char* end;
        auto input_filter = strtol(var_val, &end, 10);
        if (!*end)
            set_level(input_filter);
    }

    return *this;
}

logger& logger::set_details(int filter)
{
    // clang-format off
    static const int max_filter = static_cast<int>(details::without_app_name) +
                                  static_cast<int>(details::without_time) +
                                  static_cast<int>(details::without_microseconds) +
                                  static_cast<int>(details::without_level) +
                                  static_cast<int>(details::without_thread_info) +
                                  static_cast<int>(details::without_source_code);
    // clang-format on
    SRV_ASSERT(filter >= 0 && filter <= max_filter);

    _details_filter = filter;
    return *this;
}

logger& logger::set_details_from_environment(const char* var_name)
{
    char* var_val = getenv(var_name);
    if (var_val != NULL)
    {
        char* end;
        auto input_filter = strtol(var_val, &end, 10);
        if (!*end)
            set_details(input_filter);
    }

    return *this;
}

void logger::lock() { _logs_on = false; }

void logger::unlock() { _logs_on = true; }

logger& logger::add_destination(log_handler_type&& handler)
{
    _appenders.push_back(std::move(handler));
    return *this;
}

void logger::write(log_message& msg)
{
    if (!_logs_on.load())
        return;

    try
    {
        auto _lv = static_cast<int>(msg.context.lv);
        if (!_lv || ~_level_filter & _lv)
        {
            for (const auto& appender : _appenders)
            {
                appender(msg, _details_filter);
            }
        }
    }
    catch (std::exception& e)
    {
        SRV_TRACE_SIGNAL(e.what());
    }
}

} // namespace server_lib
