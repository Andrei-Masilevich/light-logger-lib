#pragma once

#include <boost/test/unit_test.hpp>

#include <string>

namespace ll {
namespace tests {

    template <typename Stream>
    void dump_str(Stream& s, const std::string& str)
    {
        s << ">>>\n";
        s << str << '\n';
        s << "<<<\n";
        s << std::flush;
    }

    std::string current_test_name();
    void print_current_test_name();

} // namespace tests
} // namespace ll

#ifdef NDEBUG
#define DUMP_STR(str) (str)
#else
#include <iostream>

#define DUMP_STR(str) \
    ll::tests::dump_str(std::cerr, str)
#endif
