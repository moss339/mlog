cmake_minimum_required(VERSION 3.14)

option(BUILD_SHARED_LIBS "Build shared libraries" OFF)

# Collect all source files
file(GLOB MLOG_SOURCES 
    ${CMAKE_CURRENT_SOURCE_DIR}/*.cpp
)

if(BUILD_SHARED_LIBS)
    add_library(mlog SHARED ${MLOG_SOURCES})
    set_target_properties(mlog PROPERTIES
        VERSION 1.0.0
        SOVERSION 1
    )
else()
    add_library(mlog STATIC ${MLOG_SOURCES})
    set_target_properties(mlog PROPERTIES
        VERSION 1.0.0
        SOVERSION 1
    )
endif()

target_include_directories(mlog PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/../include
    ${CMAKE_CURRENT_BINARY_DIR}/..
)

target_link_libraries(mlog PUBLIC
    Threads::Threads
)
