#include "tests_common.h"

#include <logger/ll.h>

#include <logger/platform_config.h>
#include <logger/time_helper.h>
#include <logger/asserts.h>

#if defined(SERVER_LIB_PLATFORM_LINUX)
#include <unistd.h>
#endif

#include <fstream>
#include <boost/filesystem.hpp>
#include <memory>

namespace ll {
namespace tests {

    using server_lib::to_iso_string;

    class logger_cleanup
    {
    public:
        logger_cleanup() = default;

        ~logger_cleanup()
        {
            logger::destroy();
            close_log_file();
            if (!_temp_to_log.empty())
            {
                boost::filesystem::remove(_temp_to_log);
            }
        }

        void create_log_file(const std::string& test_name)
        {
            std::string file_name = boost::filesystem::unique_path().generic_string();
            file_name += '.';
            file_name += test_name;

            _temp_to_log = boost::filesystem::temp_directory_path() / file_name;

            _tmp_file = std::unique_ptr<std::ofstream>(new std::ofstream(_temp_to_log.generic_string().c_str()));
            std::cout.clear();
            std::cout.rdbuf(_tmp_file->rdbuf());
        }

        std::string close_log_file()
        {
            if (_tmp_file)
            {
                _tmp_file->close();
                _tmp_file.reset();
                std::cout.rdbuf(pcout_old_buf);
                return _temp_to_log.generic_string();
            }
            return {};
        }

    private:
        boost::filesystem::path _temp_to_log;
        std::unique_ptr<std::ofstream> _tmp_file;
        static std::streambuf* pcout_old_buf;
    };

    std::streambuf* logger_cleanup::pcout_old_buf = std::cout.rdbuf();

    BOOST_FIXTURE_TEST_SUITE(logger_tests, logger_cleanup)

    BOOST_AUTO_TEST_CASE(default_level_check)
    {
        print_current_test_name();

        logger::instance().init_cli_log();

        create_log_file(current_test_name());

        LOG_TRACE(current_test_name() << " message");
        LOG_DEBUG(current_test_name() << " message");
        LOG_INFO(current_test_name() << " message");
        LOG_WARN(current_test_name() << " message");
        LOG_ERROR(current_test_name() << " message");
        LOG_FATAL(current_test_name() << " message");

        std::ifstream input(close_log_file());

        size_t rows = 0;
        for (std::string line; std::getline(input, line);)
        {
            ++rows;
        }

        BOOST_REQUIRE_EQUAL(rows, 6);
    }

// TODO: issue: We got empty file after the first success std::cout substitution :((
//              tests work fine only one by one.
//              Are these fucking std::streambuf private flags???
#if 0
    BOOST_AUTO_TEST_CASE(short_format_check)
    {
        print_current_test_name();

        logger::instance().init_cli_log().set_level(logger::level_debug).set_details(logger::details_message_with_level);

        create_log_file(current_test_name());

        static const std::string message = " message";
        LOG_TRACE(current_test_name() << message);
        LOG_DEBUG(current_test_name() << message);
        LOG_INFO(current_test_name() << message);
        LOG_WARN(current_test_name() << message);
        LOG_ERROR(current_test_name() << message);
        LOG_FATAL(current_test_name() << message);

        std::ifstream input(close_log_file());

        size_t rows = 0;
        for (std::string line; std::getline(input, line);)
        {
            switch (rows)
            {
            case 0:
            {
                BOOST_REQUIRE(line.find("[debug]") != std::string::npos);
                break;
            }
            case 1:
            {
                BOOST_REQUIRE(line.find("[info]") != std::string::npos);
                break;
            }
            case 2:
            {
                BOOST_REQUIRE(line.find("[warning]") != std::string::npos);
                break;
            }
            case 3:
            {
                BOOST_REQUIRE(line.find("[error!]") != std::string::npos);
                break;
            }
            case 4:
            {
                BOOST_REQUIRE(line.find("[fatal!!!]") != std::string::npos);
                break;
            }
            default:;
            }

            ++rows;
        }

        BOOST_REQUIRE_EQUAL(rows, 5);
    }

    BOOST_AUTO_TEST_CASE(issue_only_level_check)
    {
        print_current_test_name();

        logger::instance().init_cli_log().set_level(logger::level_warning);

        create_log_file(current_test_name());

        static const std::string message = " message";
        LOG_TRACE(current_test_name() << message);
        LOG_DEBUG(current_test_name() << message);
        LOG_INFO(current_test_name() << message);
        LOG_WARN(current_test_name() << message);
        LOG_ERROR(current_test_name() << message);
        LOG_FATAL(current_test_name() << message);

        std::ifstream input(close_log_file());

        size_t rows = 0;
        for (std::string line; std::getline(input, line);)
        {
            switch (rows)
            {
            case 0:
            {
                BOOST_REQUIRE(line.find("[warning]") != std::string::npos);
                break;
            }
            case 1:
            {
                BOOST_REQUIRE(line.find("[error!]") != std::string::npos);
                break;
            }
            case 2:
            {
                BOOST_REQUIRE(line.find("[fatal!!!]") != std::string::npos);
                break;
            }
            default:;
            }

            ++rows;
        }

        BOOST_REQUIRE_EQUAL(rows, 3);
    }

    BOOST_AUTO_TEST_CASE(all_details_check)
    {
        print_current_test_name();

        static const char* time_format = "%Y%m%d";

        logger::instance().init_cli_log(time_format).set_level(logger::level_trace).set_details(logger::details_all);

        create_log_file(current_test_name());

        using std::chrono::system_clock;
        auto now = system_clock::now();

        auto now_date = to_iso_string(system_clock::to_time_t(now), time_format, false);
        std::string app_name;
#if defined(SERVER_LIB_PLATFORM_LINUX)
        std::ifstream("/proc/self/comm") >> app_name;
#endif

        static const std::string message = " message";
        LOG_TRACE(current_test_name() << message);
        LOG_DEBUG(current_test_name() << message);
        LOG_INFO(current_test_name() << message);
        LOG_WARN(current_test_name() << message);
        LOG_ERROR(current_test_name() << message);
        LOG_FATAL(current_test_name() << message);

        std::ifstream input(close_log_file());

        size_t rows = 0;
        for (std::string line; std::getline(input, line);)
        {
            if (!app_name.empty())
            {
                BOOST_REQUIRE(line.find(app_name) != std::string::npos);
            }
            BOOST_REQUIRE(line.find(message) != std::string::npos);

            switch (rows)
            {
            case 0:
            {
                BOOST_REQUIRE(line.find("[trace]") != std::string::npos);
                break;
            }
            case 1:
            {
                BOOST_REQUIRE(line.find("[debug]") != std::string::npos);
                break;
            }
            case 2:
            {
                BOOST_REQUIRE(line.find("[info]") != std::string::npos);
                break;
            }
            case 3:
            {
                BOOST_REQUIRE(line.find("[warning]") != std::string::npos);
                break;
            }
            case 4:
            {
                BOOST_REQUIRE(line.find("[error!]") != std::string::npos);
                break;
            }
            case 5:
            {
                BOOST_REQUIRE(line.find("[fatal!!!]") != std::string::npos);
                break;
            }
            default:;
            }

            BOOST_REQUIRE(line.find(now_date) != std::string::npos);
            BOOST_REQUIRE(line.find(__FILE__) != std::string::npos);

            ++rows;
        }

        BOOST_REQUIRE_EQUAL(rows, 6);
    }
#endif

    BOOST_AUTO_TEST_SUITE_END()

} // namespace tests
} // namespace ll
