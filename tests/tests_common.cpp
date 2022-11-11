#include "tests_common.h"

#include <boost/filesystem.hpp>

#include <sstream>

#include <cstring>
#include <set>


namespace ll {
namespace tests {

    std::string current_test_name()
    {
        return boost::unit_test::framework::current_test_case().p_name;
    }

    void print_current_test_name()
    {
        static uint32_t test_counter = 0;

        std::stringstream ss;

        ss << "TEST (";
        ss << ++test_counter;
        ss << ") - [";
        ss << current_test_name();
        ss << "]";
        DUMP_STR(ss.str());
    }

} // namespace tests
} // namespace ll
