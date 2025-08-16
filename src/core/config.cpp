#include "core/config.hpp"
#include "nlohmann/json.hpp"
#include <fstream>
#include <stdexcept>

using json = nlohmann::json;

namespace https_server {

ServerConfig Config::load(const std::string& filename) {
    ServerConfig config;
    
    std::ifstream file(filename);
    if (!file.is_open()) {
        throw std::runtime_error("Cannot open config file: " + filename);
    }
    
    json j;
    file >> j;
    
    if (j.contains("port")) config.port = j["port"];
    if (j.contains("threads")) config.threads = j["threads"];
    if (j.contains("cert_file")) config.cert_file = j["cert_file"];
    if (j.contains("key_file")) config.key_file = j["key_file"];
    if (j.contains("web_root")) config.web_root = j["web_root"];
    
    if (j.contains("log_level")) {
        const std::string level = j["log_level"];
        if (level == "Debug") config.log_level = LogLevel::Debug;
        else if (level == "Info") config.log_level = LogLevel::Info;
        else if (level == "Warning") config.log_level = LogLevel::Warning;
        else if (level == "Error") config.log_level = LogLevel::Error;
    }
    
    return config;
}

} // namespace https_server