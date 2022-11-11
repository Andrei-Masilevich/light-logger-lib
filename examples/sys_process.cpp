#include <logger/ll.h>

#include <thread>
#include <chrono>

#define LOG_CONTEXT "SYS> " << LOG_FUNCTION_NAME << ": "

void test_subfunction();

void test_function()
{
    LOGC_TRACE("Start Payload #1");
    std::this_thread::sleep_for(std::chrono::milliseconds(111));
    test_subfunction();
    LOGC_TRACE("End Payload #1");
}

void test_subfunction()
{
    LOGC_TRACE("Start Payload #2");
    std::this_thread::sleep_for(std::chrono::milliseconds(555));
    LOGC_WARN("Smth");
    LOGC_TRACE("End Payload #2");
}

int main(int argc, char* argv[])
{
    // clang-format off
    ll::logger::instance().init_cli_log().init_sys_log()
            .set_level_from_environment("LOG_LEVEL")
            .set_details_from_environment("LOG_DETAILS");
    // clang-format on

    LOGC_DEBUG("Start");
    LOGC_INFO("Sys Hello");

    test_function();

    LOGC_DEBUG("End");

    return 0;
}
