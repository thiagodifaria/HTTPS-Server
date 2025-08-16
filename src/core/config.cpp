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
    
    if (j.contains("client_ca_file")) config.client_ca_file = j["client_ca_file"];
    
    if (j.contains("log_level")) {
        const std::string level = j["log_level"];
        if (level == "Debug") config.log_level = LogLevel::Debug;
        else if (level == "Info") config.log_level = LogLevel::Info;
        else if (level == "Warning") config.log_level = LogLevel::Warning;
        else if (level == "Error") config.log_level = LogLevel::Error;
    }
    
    if (j.contains("security")) {
        const auto& security = j["security"];
        
        if (security.contains("enable_hsts")) {
            config.security.enable_hsts = security["enable_hsts"];
        }
        if (security.contains("enable_csp")) {
            config.security.enable_csp = security["enable_csp"];
        }
        if (security.contains("enable_xcto")) {
            config.security.enable_xcto = security["enable_xcto"];
        }
        if (security.contains("enable_xfo")) {
            config.security.enable_xfo = security["enable_xfo"];
        }
        if (security.contains("hsts_max_age")) {
            config.security.hsts_max_age = security["hsts_max_age"];
        }
        if (security.contains("hsts_include_subdomains")) {
            config.security.hsts_include_subdomains = security["hsts_include_subdomains"];
        }
        if (security.contains("hsts_preload")) {
            config.security.hsts_preload = security["hsts_preload"];
        }
        if (security.contains("csp_policy")) {
            config.security.csp_policy = security["csp_policy"];
        }
    }
    
    return config;
}

} // namespace https_server