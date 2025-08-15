#ifndef HTTPS_SERVER_CONFIG_HPP
#define HTTPS_SERVER_CONFIG_HPP

#include "utils/logger.hpp"
#include <string>
#include <cstdint>

namespace https_server {

struct ServerConfig {
    std::uint16_t port = 8443;
    std::uint32_t threads = 0; // 0 = hardware_concurrency
    std::string cert_file = "cert.pem";
    std::string key_file = "key.pem";
    std::string web_root = "wwwroot";
    LogLevel log_level = LogLevel::INFO;
};

class Config {
public:
    static ServerConfig load(const std::string& filename = "config.json");
    
private:
    Config() = delete;
};

} // namespace https_server

#endif // HTTPS_SERVER_CONFIG_HPP