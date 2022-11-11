#pragma once

#include <iosfwd>
#include <set>
#include <map>
#include <vector>
#include <array>
#include <string>
#include <sstream>

#include "logger.h"
#include "macro.h"
#include "platform_config.h"

#ifndef LOG_FUNCTION_NAME
#if defined(SERVER_LIB_PLATFORM_WINDOWS)
#define LOG_FUNCTION_NAME __FUNCTION__
#else //*NIX
#define LOG_FUNCTION_NAME __func__
#endif
#endif

namespace server_lib {

class trim_file_path
{
    std::string _file;

public:
    trim_file_path(const char* file)
        : _file(file)
    {
#if defined(APPLICATION_SOURCE_DIR)
#if defined(SERVER_LIB_PLATFORM_WINDOWS)
        std::replace(m_file.begin(), _file.end(), '\\', '/');
#endif
        auto start_pos = _file.find(APPLICATION_SOURCE_DIR);

        if (start_pos != std::string::npos)
            _file.erase(start_pos, sizeof(APPLICATION_SOURCE_DIR));
#endif
    }

    operator std::string() { return _file; }
};

#define SRV_LOG_NS_ server_lib

#define LOG_LOG(LEVEL, FILE, LINE, FUNC, ARG)                     \
    SRV_EXPAND_MACRO(                                             \
        SRV_MULTILINE_MACRO_BEGIN {                               \
            SRV_LOG_NS_::logger::log_message msg;                 \
            msg.context.lv = LEVEL;                               \
            msg.context.file = SRV_LOG_NS_::trim_file_path(FILE); \
            msg.context.line = LINE;                              \
            msg.context.method = FUNC;                            \
            msg.message << ARG;                                   \
            SRV_LOG_NS_::logger::instance().write(msg);           \
        } SRV_MULTILINE_MACRO_END)

#define LOG_TRACE(ARG) LOG_LOG(SRV_LOG_NS_::logger::level::trace, __FILE__, __LINE__, LOG_FUNCTION_NAME, ARG)
#define LOG_DEBUG(ARG) LOG_LOG(SRV_LOG_NS_::logger::level::debug, __FILE__, __LINE__, LOG_FUNCTION_NAME, ARG)
#define LOG_INFO(ARG) LOG_LOG(SRV_LOG_NS_::logger::level::info, __FILE__, __LINE__, LOG_FUNCTION_NAME, ARG)
#define LOG_WARN(ARG) LOG_LOG(SRV_LOG_NS_::logger::level::warning, __FILE__, __LINE__, LOG_FUNCTION_NAME, ARG)
#define LOG_ERROR(ARG) LOG_LOG(SRV_LOG_NS_::logger::level::error, __FILE__, __LINE__, LOG_FUNCTION_NAME, ARG)
#define LOG_FATAL(ARG) LOG_LOG(SRV_LOG_NS_::logger::level::fatal, __FILE__, __LINE__, LOG_FUNCTION_NAME, ARG)

#define LOGC_TRACE(ARG) LOG_TRACE(LOG_CONTEXT << ARG)
#define LOGC_DEBUG(ARG) LOG_DEBUG(LOG_CONTEXT << ARG)
#define LOGC_INFO(ARG) LOG_INFO(LOG_CONTEXT << ARG)
#define LOGC_WARN(ARG) LOG_WARN(LOG_CONTEXT << ARG)
#define LOGC_ERROR(ARG) LOG_ERROR(LOG_CONTEXT << ARG)
#define LOGC_FATAL(ARG) LOG_FATAL(LOG_CONTEXT << ARG)

} // namespace server_lib
