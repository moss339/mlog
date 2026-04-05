cmake_minimum_required(VERSION 3.14)

option(BUILD_SHARED_LIBS "Build shared libraries" ON)

project(mlog VERSION 1.0.0 LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

find_package(Threads REQUIRED)

include(CMakePackageConfigHelpers)
include(GNUInstallDirs)

set(CJSON_ROOT "${CMAKE_CURRENT_SOURCE_DIR}/thirdparty/cJSON")
set(MLOG_SOURCES
    src/logger.cpp
    src/console_sink.cpp
    src/file_sink.cpp
    src/formatter.cpp
    src/async_queue.cpp
    src/config.cpp
    ${CJSON_ROOT}/cJSON.c
)

if(BUILD_SHARED_LIBS)
    add_library(mlog SHARED ${MLOG_SOURCES})
    set_target_properties(mlog PROPERTIES
        VERSION ${PROJECT_VERSION}
        SOVERSION 1
        POSITION_INDEPENDENT_CODE ON
    )
else()
    add_library(mlog STATIC ${MLOG_SOURCES})
    set_target_properties(mlog PROPERTIES
        VERSION ${PROJECT_VERSION}
        SOVERSION 1
    )
endif()

target_include_directories(mlog PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/src>
    $<BUILD_INTERFACE:${CJSON_ROOT}>
    $<INSTALL_INTERFACE:include>
)

target_link_libraries(mlog PUBLIC Threads::Threads)

# Export mlog target
export(TARGETS mlog NAMESPACE mlog:: FILE mlogTargets.cmake)

install(TARGETS mlog
    EXPORT mlogTargets
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
    PUBLIC_HEADER DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/mlog
)

install(EXPORT mlogTargets
    FILE mlogTargets.cmake
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/mlog
)

install(DIRECTORY include/mlog/ DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/mlog)

configure_package_config_file(
    ${CMAKE_CURRENT_SOURCE_DIR}/cmake/mlogConfig.cmake.in
    ${CMAKE_CURRENT_BINARY_DIR}/cmake/mlogConfig.cmake
    INSTALL_DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/mlog
)

install(FILES ${CMAKE_CURRENT_BINARY_DIR}/cmake/mlogConfig.cmake
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/mlog
)
