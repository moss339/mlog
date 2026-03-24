#include "mlog/mlog.h"
#include <thread>
#include <vector>
#include <chrono>
#include <cassert>
#include <iostream>

using namespace mlog;

void test_single_logger() {
    std::cout << "Testing single logger..." << std::endl;

    LoggerConfig config;
    config.name = "test_single";
    config.level = LogLevel::Debug;
    config.async = false;

    SinkConfig console_sink;
    console_sink.type = "console";
    console_sink.level = LogLevel::Debug;
    config.sinks.push_back(console_sink);

    auto logger = LoggerFactory::instance().create_logger(config);

    logger->info("Single logger test message");
    logger->debug("Debug message");
    logger->warn("Warning message");
    logger->error("Error message");

    std::cout << "Single logger test passed!" << std::endl;
}

void test_multi_instance() {
    std::cout << "Testing multi-instance..." << std::endl;

    LoggerConfig config1;
    config1.name = "logger1";
    config1.level = LogLevel::Info;
    config1.async = false;
    SinkConfig sink1;
    sink1.type = "console";
    sink1.level = LogLevel::Info;
    config1.sinks.push_back(sink1);

    LoggerConfig config2;
    config2.name = "logger2";
    config2.level = LogLevel::Debug;
    config2.async = false;
    SinkConfig sink2;
    sink2.type = "console";
    sink2.level = LogLevel::Debug;
    config2.sinks.push_back(sink2);

    auto logger1 = LoggerFactory::instance().create_logger(config1);
    auto logger2 = LoggerFactory::instance().create_logger(config2);

    logger1->info("Message from logger1");
    logger2->debug("Debug from logger2");

    std::cout << "Multi-instance test passed!" << std::endl;
}

void test_multithread() {
    std::cout << "Testing multithread writing..." << std::endl;

    LoggerConfig config;
    config.name = "test_multithread";
    config.level = LogLevel::Debug;
    config.async = true;
    config.queue_capacity = 4096;

    SinkConfig sink;
    sink.type = "console";
    sink.level = LogLevel::Debug;
    config.sinks.push_back(sink);

    auto logger = LoggerFactory::instance().create_logger(config);

    std::vector<std::thread> threads;
    int num_threads = 4;
    int msgs_per_thread = 100;

    for (int t = 0; t < num_threads; ++t) {
        threads.emplace_back([logger, t, msgs_per_thread]() {
            for (int i = 0; i < msgs_per_thread; ++i) {
                logger->info("Thread {} message {}", t, i);
                std::this_thread::sleep_for(std::chrono::milliseconds(1));
            }
        });
    }

    for (auto& t : threads) {
        t.join();
    }

    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    logger->flush();

    std::cout << "Multithread test passed! Sent " << num_threads * msgs_per_thread << " messages." << std::endl;
}

void test_log_levels() {
    std::cout << "Testing log levels..." << std::endl;

    LoggerConfig config;
    config.name = "test_levels";
    config.level = LogLevel::Warn;
    config.async = false;

    SinkConfig sink;
    sink.type = "console";
    sink.level = LogLevel::Warn;
    config.sinks.push_back(sink);

    auto logger = LoggerFactory::instance().create_logger(config);

    logger->trace("This trace should not appear");
    logger->debug("This debug should not appear");
    logger->info("This info should not appear");
    logger->warn("This warn should appear");
    logger->error("This error should appear");

    std::cout << "Log levels test passed!" << std::endl;
}

void test_fmt_formatting() {
    std::cout << "Testing fmt formatting..." << std::endl;

    LoggerConfig config;
    config.name = "test_fmt";
    config.level = LogLevel::Debug;
    config.async = false;

    SinkConfig sink;
    sink.type = "console";
    sink.level = LogLevel::Debug;
    config.sinks.push_back(sink);

    auto logger = LoggerFactory::instance().create_logger(config);

    logger->info("Integer: {}, String: {}, Float: {:.2f}", 42, "hello", 3.14159);
    logger->debug("Multiple args: {} {} {} {}", 1, 2, 3, 4);

    std::cout << "Fmt formatting test passed!" << std::endl;
}

void test_logger_factory() {
    std::cout << "Testing LoggerFactory..." << std::endl;

    LoggerFactory::instance().clear();

    LoggerConfig config;
    config.name = "factory_test";
    config.level = LogLevel::Info;
    config.async = false;

    auto logger1 = LoggerFactory::instance().create_logger(config);
    auto logger2 = LoggerFactory::instance().get_logger("factory_test");

    assert(logger1.get() == logger2.get());

    auto logger3 = LoggerFactory::instance().get_logger("nonexistent");
    assert(logger3 == nullptr);

    std::cout << "LoggerFactory test passed!" << std::endl;
}

int main() {
    std::cout << "=== MLog Test Suite ===" << std::endl;

    test_single_logger();
    test_multi_instance();
    test_multithread();
    test_log_levels();
    test_fmt_formatting();
    test_logger_factory();

    std::cout << "\n=== All tests passed! ===" << std::endl;
    return 0;
}
