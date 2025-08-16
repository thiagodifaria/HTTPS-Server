#ifndef HTTPS_SERVER_CONFIG_HPP
#define HTTPS_SERVER_CONFIG_HPP

#include "utils/logger.hpp"
#include <string>
#include <cstdint>

namespace https_server {

struct SecurityConfig {
    bool enable_hsts = true;
    bool enable_csp = true;
    bool enable_xcto = true;
    bool enable_xfo = true;
    std::string hsts_max_age = "31536000";
    bool hsts_include_subdomains = true;
    bool hsts_preload = false;
    std::string csp_policy = "default-src 'self'; script-src 'self' 'unsafe-inline'; style-src 'self' 'unsafe-inline'; img-src 'self' data:; font-src 'self'";
};

struct ServerConfig {
    std::uint16_t port = 8443;
    std::uint32_t threads = 0;
    std::string cert_file = "cert.pem";
    std::string key_file = "key.pem";
    std::string web_root = "public";
    LogLevel log_level = LogLevel::Info;
    
    std::string client_ca_file = "";
    
    SecurityConfig security;
};

class Config {
public:
    static ServerConfig load(const std::string& filename = "config.json");
    
private:
    Config() = delete;
};

} // namespace https_server

#endif // HTTPS_SERVER_CONFIG_HPP