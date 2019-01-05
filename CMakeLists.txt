cmake_minimum_required(VERSION 3.6)
project(ivy)
set(CMAKE_CXX_STANDARD 14)

link_directories(/usr/local/lib)

set(IVY_MASTER_KEY "default key has a length of 32b" CACHE STRING "Define you master key here")
add_definitions(-DIVY_MASTER_KEY="${IVY_MASTER_KEY}")

set(IVY_SERVER_PORT "1973" CACHE STRING "Server port")
add_definitions(-DIVY_SERVER_PORT=${IVY_SERVER_PORT})

set(IVY_SERVER_LOGGER_PATH "./logs" CACHE STRING "Server logging path")
add_definitions(-DIVY_SERVER_LOGGER_PATH="${IVY_SERVER_LOGGER_PATH}")
add_library(compact_logger server/compact_logger.cpp server/compact_logger.h)
target_link_libraries(compact_logger glog)

add_library(crypto crypto.cpp crypto.h)
target_link_libraries(crypto cryptopp)

add_executable(server server/main.cpp protocol.h server/packet_handler.cpp server/packet_handler.h server/logging_formats.h server/logging_util.h)
target_link_libraries(server glog crypto compact_logger)

add_executable(controller controller/main.cpp)
target_link_libraries(controller atomic)

add_library(receiver controller/receiver.cpp controller/receiver.h)

add_library(messages controller/messages/raw.cpp controller/messages/raw.h controller/util.h)

find_package(CxxTest)
if (CXXTEST_FOUND)
    # Add logs for test_compact_logger
    file(MAKE_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/logs)

    include_directories(${CXXTEST_INCLUDE_DIR})
    enable_testing()

    CXXTEST_ADD_TEST(test_crypto test_crypto.gen.cpp ${CMAKE_CURRENT_SOURCE_DIR}/test/test_crypto.cpp)
    target_link_libraries(test_crypto crypto)

    CXXTEST_ADD_TEST(test_protocol test_protocol.gen.cpp ${CMAKE_CURRENT_SOURCE_DIR}/test/test_protocol.cpp)

    CXXTEST_ADD_TEST(test_compact_logger test_compact_logger.gen.cpp ${CMAKE_CURRENT_SOURCE_DIR}/test/server/test_compact_logger.cpp)
    target_link_libraries(test_compact_logger compact_logger)
    add_custom_command(
            TARGET test_compact_logger POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy
            ${CMAKE_SOURCE_DIR}/test/pre_logger_test.sh
            ${CMAKE_CURRENT_BINARY_DIR}/pre_logger_test.sh)

    CXXTEST_ADD_TEST(test_util test_util.gen.cpp ${CMAKE_CURRENT_SOURCE_DIR}/test/controller/test_util.cpp)
endif ()
