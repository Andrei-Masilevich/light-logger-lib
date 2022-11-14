#pragma once

#include <functional>
#include <string>
#include <sstream>
#include <vector>
#include <atomic>
#include <cstdint>
#include <utility>
#include <mutex>
#include <tuple>

#include "singleton.h"

namespace server_lib {

class logger : public singleton<logger>
{
public:
    static const char* default_time_format;

    enum class level
    {
        fatal = 0x0, //not filtered
        error = 0x1,
        warning = 0x2,
        info = 0x4,
        debug = 0x8,
        trace = 0x10,
    };

    static const int level_trace; // 0
    static const int level_debug; // 16
    static const int level_info; // 24
    static const int level_warning; // 28
    static const int level_error; // 30

    enum class details
    {
        all = 0x0, //not filtered
        without_app_name = 0x1,
        without_time = 0x2,
        without_microseconds = 0x4,
        without_level = 0x8,
        without_thread_info = 0x10,
        without_source_code = 0x20,
    };

    static const int details_all; // 0
    static const int details_without_app_name; // 1
    static const int details_message_with_level; // 55
    static const int details_message_without_source_code; // 33
    static const int details_message_only; // 63

    struct log_context
    {
        unsigned long id;
        level lv;
        std::string file;
        int line;
        std::string method;

        using thread_info_type = std::tuple<uint64_t, std::string, bool>;
        thread_info_type thread_info;

        log_context();

    private:
        static std::atomic_ulong s_id_counter;
    };

    struct log_message
    {
        log_context context;
        std::stringstream message;
    };

    using log_handler_type = std::function<void(const log_message&, int details_filter)>;

protected:
    logger();

    friend class singleton<logger>;

public:
    logger& init_cli_log(const char* time_format = logger::default_time_format);
    logger& init_sys_log();

    logger& set_level(int filter = logger::level_debug);
    logger& set_level_from_environment(const char* var_name);

    int level_filter() const
    {
        return _level_filter;
    }

    logger& set_details(int filter = logger::details_without_app_name);
    logger& set_details_from_environment(const char* var_name);

    int details_filter() const
    {
        return _details_filter;
    }

    void lock();
    void unlock();

    logger& add_destination(log_handler_type&& handler);

    void write(log_message& msg);

    size_t get_appenders_count() const
    {
        return _appenders.size();
    }

private:
    void add_cli_destination();
    void add_syslog_destination();

private:
    std::vector<log_handler_type> _appenders;
    bool _added_cli_destination = false;
    bool _added_syslog_destination = false;

    std::string _time_format = logger::default_time_format;
    int _level_filter = logger::level_trace;
    int _details_filter = logger::details_without_app_name;
    std::atomic_bool _logs_on;
    std::mutex _mutex_for_row;
};

} // namespace server_lib
