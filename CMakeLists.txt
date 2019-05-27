cmake_minimum_required(VERSION 3.6)
project(ivy)
set(CMAKE_CXX_STANDARD 14)

find_package(glog REQUIRED)
find_package(gflags REQUIRED)
find_package(folly REQUIRED)
set(GLOG glog gflags)
set(FOLLY folly boost_system boost_regex double-conversion dl boost_program_options boost_filesystem pthread iberty)

find_package(cryptopp REQUIRED)

link_directories(/usr/local/lib)

set(IVY_MASTER_KEY "default key has a length of 32b" CACHE STRING "Define you master key here")
add_definitions(-DIVY_MASTER_KEY="${IVY_MASTER_KEY}")

set(IVY_SERVER_PORT "1973" CACHE STRING "Server port")
add_definitions(-DIVY_SERVER_PORT=${IVY_SERVER_PORT})

set(IVY_SERVER_LOGGER_PATH "./logs" CACHE STRING "Server logging path")
add_definitions(-DIVY_SERVER_LOGGER_PATH="${IVY_SERVER_LOGGER_PATH}")
add_library(compact_logger server/compact_logger.cpp server/compact_logger.h)
target_link_libraries(compact_logger ${GLOG})

add_library(crypto crypto.cpp crypto.h)
target_link_libraries(crypto cryptopp)

add_executable(server server/main.cpp protocol.h server/packet_handler.cpp server/packet_handler.h server/logging_formats.h server/logging_util.h)
target_link_libraries(server ${GLOG} crypto compact_logger)

add_executable(controller controller/main.cpp)
target_link_libraries(controller atomic)

add_library(messages controller/messages/raw.cpp controller/messages/raw.h controller/util/ip_id.h)

add_library(receiver controller/udp_receiver.cpp controller/udp_receiver.h controller/tcp_receiver.cpp controller/tcp_receiver.h controller/controller.h)
target_link_libraries(receiver ${GLOG} messages ${FOLLY} scope_guard)

add_library(sender controller/sender.cpp controller/sender.h controller/controller.h)
target_link_libraries(sender ${GLOG} messages ${FOLLY} scope_guard)

add_library(scope_guard controller/util/scope_guard.cpp controller/util/scope_guard.h)

if (EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/cxxtest)
    set(CXXTEST_INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/cxxtest)
    set(CXXTEST_PYTHON_TESTGEN_EXECUTABLE ${CMAKE_CURRENT_SOURCE_DIR}/cxxtest/bin/cxxtestgen)
endif ()
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

    CXXTEST_ADD_TEST(test_ip_id test_ip_id.gen.cpp ${CMAKE_CURRENT_SOURCE_DIR}/test/controller/util/test_ip_id.cpp)

    CXXTEST_ADD_TEST(test_receiver_sender test_receiver_sender.gen.cpp ${CMAKE_CURRENT_SOURCE_DIR}/test/controller/test_receiver_sender.cpp)
    target_link_libraries(test_receiver_sender receiver sender)

    CXXTEST_ADD_TEST(test_scope_guard test_scope_guard.gen.cpp ${CMAKE_CURRENT_SOURCE_DIR}/test/controller/util/test_scope_guard.cpp)
    target_link_libraries(test_scope_guard scope_guard)
endif ()
