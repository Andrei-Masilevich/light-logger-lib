#include <logger/time_helper.h>
#include <logger/platform_config.h>

#if defined(SERVER_LIB_PLATFORM_MOBILE)
#include <cstdlib>
#include <ctime>
#include <time.h> //POSIX strptime
#else //< SERVER_LIB_PLATFORM_MOBILE
#include <ctime>
#include <iomanip> // std::put_time, std::get_time
#include <sstream>
#endif //< !SERVER_LIB_PLATFORM_MOBILE
#include <chrono>


namespace server_lib {

#if !defined(SERVER_LIB_PLATFORM_MOBILE)
#if !defined(SERVER_LIB_PLATFORM_WINDOWS)
time_t from_iso_string(const std::string& formatted, const char* format, bool should_utc)
{
    std::stringstream ss;

    ss << formatted;

    std::tm tp {};

    ss >> std::get_time(&tp, format);
    if (!ss.fail())
    {
        time_t r = std::mktime(&tp);
        return (should_utc) ? (r + tp.tm_gmtoff) : (r);
    }
    return {};
}
#else //< !SERVER_LIB_PLATFORM_WINDOWS
time_t from_iso_string(const std::string& formatted, const char* format, bool should_utc)
{
    std::stringstream ss;

    ss << formatted;

    std::tm tp {};

    ss >> std::get_time(&tp, format);
    if (!ss.fail())
    {
        return (should_utc) ? _mkgmtime(&tp) : std::mktime(&tp);
    }
    return {};
}
#endif //< SERVER_LIB_PLATFORM_WINDOWS

std::string to_iso_string(const time_t t, const char* format, bool should_utc)
{
    std::stringstream ss;

    ss << std::put_time((should_utc) ? std::gmtime(&t) : std::localtime(&t), format);

    return ss.str();
}
#else //< !SERVER_LIB_PLATFORM_MOBILE
time_t from_iso_string(const std::string& formatted, const char* format, bool should_utc)
{
    std::tm tp {};

    auto call_r = strptime(formatted.c_str(), format, &tp);

    SRV_ASSERT(call_r != NULL, "Can't parse time");

    time_t r = std::mktime(&tp);
    return (should_utc) ? (r + tp.tm_gmtoff) : (r);
}

std::string to_iso_string(const time_t t, const char* format, bool should_utc)
{
    char buff[100];

    auto call_r = std::strftime(buff, sizeof(buff), format,
                                (should_utc) ? std::gmtime(&t) : std::localtime(&t));

    SRV_ASSERT(call_r > 0, "Can't format time");

    return { buff };
}
#endif //< SERVER_LIB_PLATFORM_MOBILE

} // namespace server_lib
