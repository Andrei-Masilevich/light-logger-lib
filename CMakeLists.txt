project( LightLoggerLib )
cmake_minimum_required( VERSION 3.0.2 )

set(CMAKE_CXX_STANDARD 11)

find_package(Threads REQUIRED)

set(LIB_SOURCES
    "${CMAKE_CURRENT_SOURCE_DIR}/src/logger.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/logging_trace.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/time_helper.cpp"
)

file(GLOB_RECURSE LIB_IMPL_HEADERS
    "${CMAKE_CURRENT_SOURCE_DIR}/src/*.h")
file(GLOB_RECURSE LIB_HEADERS
    "${CMAKE_CURRENT_SOURCE_DIR}/include/logger/*.h")

add_library( logger_lib
             ${LIB_SOURCES}
             ${LIB_IMPL_HEADERS}
             ${LIB_HEADERS} )

set(PLATFORM_SPECIFIC_LIBS)

list(APPEND LIB_USE_LIBS
    Threads::Threads
    ${PLATFORM_SPECIFIC_LIBS})

target_link_libraries( logger_lib
              ${LIB_USE_LIBS})

target_include_directories( logger_lib
                         PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/include" )

option ( LIB_BUILD_TESTS "Build tests (ON OR OFF). This option makes sense only for integrated library!" OFF)
option ( LIB_BUILD_EXAMPLES "Build examples (ON OR OFF). This option makes sense only for integrated library!" OFF)

# If this lib is not a sub-project:
if("${CMAKE_SOURCE_DIR}" STREQUAL "${CMAKE_CURRENT_SOURCE_DIR}")
    set(LIB_BUILD_TESTS ON)
    set(LIB_BUILD_EXAMPLES ON)
endif()

if ( LIB_BUILD_TESTS )

    SET(BOOST_VERSION_MIN "1.65" CACHE STRING "Minimum required Boost Lib version")

    set(BOOST_COMPONENTS)
    list(APPEND BOOST_COMPONENTS system
                                thread
                                program_options
                                log
                                filesystem
                                date_time
                                iostreams
                                regex
                                unit_test_framework)
    set( Boost_USE_STATIC_LIBS ON CACHE STRING "ON or OFF" )
    set( Boost_USE_MULTITHREADED ON CACHE STRING "ON or OFF" )

    find_package(Boost ${BOOST_VERSION_MIN} REQUIRED COMPONENTS ${BOOST_COMPONENTS})

    include_directories( ${Boost_INCLUDE_DIR} )

    file(GLOB_RECURSE SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/tests/*.cpp")
    file(GLOB_RECURSE HEADERS "${CMAKE_CURRENT_SOURCE_DIR}/tests/*.h")

    add_executable(logger_lib_tests ${SOURCES} ${HEADERS})

    add_dependencies( logger_lib_tests
                   logger_lib)
    target_include_directories( logger_lib_tests
                        PRIVATE "${Boost_INCLUDE_DIR}")
    target_link_libraries( logger_lib_tests
                        logger_lib
                        ${Boost_LIBRARIES}
                        ${PLATFORM_SPECIFIC_LIBS})

endif()

if ( LIB_BUILD_EXAMPLES )
    add_subdirectory(examples)
endif()
