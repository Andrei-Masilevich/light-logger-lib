#include <logger/platform_config.h>
#include <logger/asserts.h>
#include <logger/macro.h>

#include <string.h>
#if defined(SERVER_LIB_PLATFORM_LINUX)
#include <unistd.h>
#endif

namespace server_lib {

namespace {
    // There is no pretty formatting functions (xprintf) that are async-signal-safe
    void __print_trace_s(char* buff, size_t sz, const char* text, int out_fd)
    {
        memset(buff, 0, sz);
        int ln = SRV_C_MIN(sz - 1, strnlen(text, sz));
        strncpy(buff, text, ln);
#if defined(SERVER_LIB_PLATFORM_LINUX)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-result"
        write(out_fd, buff, strnlen(buff, sz));
#pragma GCC diagnostic pop
#else
        SRV_ERROR(buff);
#endif // !SERVER_LIB_PLATFORM_LINUX
    }
} // namespace

void print_trace_s(const char* text, int out_fd)
{
    char buff[1024];

    __print_trace_s(buff, sizeof(buff), text, out_fd);
}

} // namespace server_lib
