cmake_minimum_required(VERSION 3.15)
project(mlog VERSION 1.0.0 LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

find_package(Threads REQUIRED)

set(CJSON_ROOT "${CMAKE_CURRENT_SOURCE_DIR}/thirdparty/cJSON")
include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${CMAKE_CURRENT_SOURCE_DIR}/src
    ${CJSON_ROOT}
)

add_library(mlog SHARED
    src/logger.cpp
    src/console_sink.cpp
    src/file_sink.cpp
    src/formatter.cpp
    src/async_queue.cpp
    src/config.cpp
    ${CJSON_ROOT}/cJSON.c
)

target_include_directories(mlog PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>
)

target_link_libraries(mlog PUBLIC
    Threads::Threads
)

set_target_properties(mlog PROPERTIES
    VERSION ${PROJECT_VERSION}
    SOVERSION 1
    PUBLIC_HEADER "include/mlog/mlog.h"
)

option(MLOG_BUILD_TESTS "Build mlog tests" OFF)
option(MLOG_BUILD_EXAMPLES "Build mlog examples" ON)

if(MLOG_BUILD_TESTS)
    add_executable(test_logger
        test/test_logger.cpp
    )
    target_include_directories(test_logger PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/include
    )
    target_link_libraries(test_logger PRIVATE
        mlog
        Threads::Threads
    )
endif()

if(MLOG_BUILD_EXAMPLES)
    add_executable(basic_example
        example/basic_example.cpp
    )
    target_include_directories(basic_example PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/include
    )
    target_link_libraries(basic_example PRIVATE
        mlog
        Threads::Threads
    )
endif()

install(TARGETS mlog
    LIBRARY DESTINATION lib
    PUBLIC_HEADER DESTINATION include
)
