#include "mlog/mlog.h"
#include <thread>
#include <chrono>
#include <iostream>

using namespace mlog;

int main() {
    std::cout << "=== MLog Basic Example ===" << std::endl;

    // Create logger configuration
    LoggerConfig config;
    config.name = "basic_example";
    config.level = LogLevel::Debug;
    config.async = true;
    config.queue_capacity = 2048;

    // Console sink
    SinkConfig console_sink;
    console_sink.type = "console";
    console_sink.level = LogLevel::Debug;
    config.sinks.push_back(console_sink);

    // File sink
    SinkConfig file_sink;
    file_sink.type = "file";
    file_sink.file_path = "/tmp/mlog_example.log";
    file_sink.level = LogLevel::Info;
    file_sink.max_file_size = 1024 * 1024; // 1MB
    file_sink.max_file_count = 3;
    config.sinks.push_back(file_sink);

    // Create logger
    auto logger = LoggerFactory::instance().create_logger(config);

    // Log messages at different levels
    logger->trace("This is a trace message");
    logger->debug("This is a debug message");
    logger->info("This is an info message");
    logger->warn("This is a warning message");
    logger->error("This is an error message");

    // Use fmt-style formatting
    logger->info("User {} logged in from {} with {} attempts", "john", "192.168.1.1", 3);
    logger->warn("Config value {} is deprecated, use {} instead", "old_param", "new_param");
    logger->error("Failed to connect to {}:{} after {} attempts", "database", 5432, 5);

    // Multi-threaded logging demonstration
    std::cout << "\nStarting multi-threaded test..." << std::endl;
    std::vector<std::thread> threads;

    for (int i = 0; i < 3; ++i) {
        threads.emplace_back([logger, i]() {
            for (int j = 0; j < 5; ++j) {
                logger->info("Thread {} - Message {}", i, j);
                std::this_thread::sleep_for(std::chrono::milliseconds(10));
            }
        });
    }

    for (auto& t : threads) {
        t.join();
    }

    // Give async queue time to process
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    logger->flush();

    std::cout << "\nExample completed! Check /tmp/mlog_example.log for file output." << std::endl;

    return 0;
}
