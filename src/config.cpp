#include "mlog/config.h"
#include <fstream>
#include <sstream>
#include <algorithm>
#include <cJSON.h>

namespace mlog {

static bool parse_log_level(cJSON* obj, LogLevel& level) {
    cJSON* level_str = cJSON_GetObjectItem(obj, "level");
    if (level_str && cJSON_IsString(level_str)) {
        level = log_level_from_string(level_str->valuestring);
        return true;
    }
    return false;
}

static bool parse_sink_config(cJSON* sink_obj, SinkConfig& sink_config) {
    cJSON* type = cJSON_GetObjectItem(sink_obj, "type");
    if (!type || !cJSON_IsString(type)) {
        return false;
    }
    sink_config.type = type->valuestring;

    if (sink_config.type == "console") {
        parse_log_level(sink_obj, sink_config.level);
        return true;
    } else if (sink_config.type == "file") {
        cJSON* file_path = cJSON_GetObjectItem(sink_obj, "file_path");
        if (!file_path || !cJSON_IsString(file_path)) {
            return false;
        }
        sink_config.file_path = file_path->valuestring;

        parse_log_level(sink_obj, sink_config.level);

        cJSON* max_size = cJSON_GetObjectItem(sink_obj, "max_file_size");
        if (max_size && cJSON_IsNumber(max_size)) {
            sink_config.max_file_size = static_cast<size_t>(max_size->valueint);
        }

        cJSON* max_count = cJSON_GetObjectItem(sink_obj, "max_file_count");
        if (max_count && cJSON_IsNumber(max_count)) {
            sink_config.max_file_count = max_count->valueint;
        }

        return true;
    }

    return false;
}

bool load_config_from_file(const std::string& path, LoggerConfig& config) {
    std::ifstream file(path);
    if (!file.is_open()) {
        return false;
    }

    std::stringstream buffer;
    buffer << file.rdbuf();
    std::string content = buffer.str();

    cJSON* root = cJSON_Parse(content.c_str());
    if (!root) {
        return false;
    }

    cJSON* name = cJSON_GetObjectItem(root, "name");
    if (name && cJSON_IsString(name)) {
        config.name = name->valuestring;
    }

    parse_log_level(root, config.level);

    cJSON* async = cJSON_GetObjectItem(root, "async");
    if (async && cJSON_IsBool(async)) {
        config.async = cJSON_IsTrue(async);
    }

    cJSON* queue_cap = cJSON_GetObjectItem(root, "queue_capacity");
    if (queue_cap && cJSON_IsNumber(queue_cap)) {
        config.queue_capacity = static_cast<size_t>(queue_cap->valueint);
    }

    cJSON* sinks_array = cJSON_GetObjectItem(root, "sinks");
    if (sinks_array && cJSON_IsArray(sinks_array)) {
        int size = cJSON_GetArraySize(sinks_array);
        for (int i = 0; i < size; ++i) {
            cJSON* sink_obj = cJSON_GetArrayItem(sinks_array, i);
            SinkConfig sink_config;
            if (parse_sink_config(sink_obj, sink_config)) {
                config.sinks.push_back(sink_config);
            }
        }
    }

    cJSON_Delete(root);
    return true;
}

} // namespace mlog
