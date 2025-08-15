#include "http/static_handler.hpp"
#include "utils/logger.hpp"
#include <fstream>
#include <sstream>
#include <filesystem>

namespace https_server {

StaticHandler::StaticHandler(const std::string& web_root) 
    : web_root_(web_root) {
    init_mime_types();
}

void StaticHandler::init_mime_types() {
    mime_types_[".html"] = "text/html";
    mime_types_[".htm"] = "text/html";
    mime_types_[".css"] = "text/css";
    mime_types_[".js"] = "application/javascript";
    mime_types_[".json"] = "application/json";
    mime_types_[".jpg"] = "image/jpeg";
    mime_types_[".jpeg"] = "image/jpeg";
    mime_types_[".png"] = "image/png";
    mime_types_[".gif"] = "image/gif";
    mime_types_[".ico"] = "image/x-icon";
    mime_types_[".txt"] = "text/plain";
}

http::HttpResponse StaticHandler::handle(const http::HttpRequest& request) {
    http::HttpResponse response;
    
    std::string file_path = request.uri.substr(7);
    if (file_path.empty() || file_path == "/") {
        file_path = "/index.html";
    }
    
    file_path = normalize_path(file_path);
    
    if (!is_safe_path(file_path)) {
        response.status_code = 403;
        response.status_text = "Forbidden";
        response.body = "<h1>403 Forbidden</h1>";
        return response;
    }
    
    const std::string full_path = web_root_ + file_path;
    
    std::ifstream file(full_path, std::ios::binary);
    if (!file.is_open()) {
        response.status_code = 404;
        response.status_text = "Not Found";
        response.body = "<h1>404 Not Found</h1>";
        return response;
    }
    
    std::stringstream buffer;
    buffer << file.rdbuf();
    response.body = buffer.str();
    
    const std::string content_type = get_content_type(file_path);
    response.headers["Content-Type"] = content_type;
    
    LOG_INFO("Served static file: " + file_path);
    
    return response;
}

std::string StaticHandler::get_content_type(const std::string& file_path) const {
    const size_t dot_pos = file_path.find_last_of('.');
    if (dot_pos == std::string::npos) {
        return "application/octet-stream";
    }
    
    const std::string ext = file_path.substr(dot_pos);
    const auto it = mime_types_.find(ext);
    return (it != mime_types_.end()) ? it->second : "application/octet-stream";
}

bool StaticHandler::is_safe_path(const std::string& requested_path) const {
    return requested_path.find("..") == std::string::npos;
}

std::string StaticHandler::normalize_path(const std::string& path) const {
    std::string normalized = path;
    for (char& c : normalized) {
        if (c == '\\') c = '/';
    }
    return normalized;
}

} // namespace https_server