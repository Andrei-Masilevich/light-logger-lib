#include <boost/test/included/unit_test.hpp>

boost::unit_test::test_suite* init_unit_test_suite(int, char*[])
{
    boost::unit_test::framework::master_test_suite().p_name.value = "Test Light-Logger library";

    return nullptr;
}
