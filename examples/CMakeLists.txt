add_executable( simple_process
                "${CMAKE_CURRENT_SOURCE_DIR}/simple_process.cpp")
add_dependencies( simple_process logger_lib )
target_link_libraries( simple_process
                       logger_lib
                       ${PLATFORM_SPECIFIC_LIBS})

target_compile_definitions( simple_process PRIVATE -DAPPLICATION_SOURCE_DIR="${CMAKE_CURRENT_SOURCE_DIR}")

add_executable( sys_process
               "${CMAKE_CURRENT_SOURCE_DIR}/sys_process.cpp")
add_dependencies( sys_process logger_lib )
target_link_libraries( sys_process
                      logger_lib
                      ${PLATFORM_SPECIFIC_LIBS})

target_compile_definitions( sys_process PRIVATE -DAPPLICATION_SOURCE_DIR="${CMAKE_CURRENT_SOURCE_DIR}")
