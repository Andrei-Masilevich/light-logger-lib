#pragma once

#include <cstdint>
#include <string>
#include <stdexcept>


namespace server_lib {

time_t from_iso_string(const std::string& formatted, const char* format, bool should_utc = true);
std::string to_iso_string(const time_t, const char* format, bool should_utc = true);

} // namespace server_lib
