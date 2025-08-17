#include "http/static_handler.hpp"
#include "utils/logger.hpp"
#include <fstream>
#include <sstream>

namespace https_server {

StaticHandler::StaticHandler(const std::string& web_root) 
    : web_root_(web_root) {
    init_mime_types();
}

void StaticHandler::init_mime_types() {
    mime_types_[".html"] = "text/html; charset=utf-8";
    mime_types_[".htm"] = "text/html; charset=utf-8";
    mime_types_[".css"] = "text/css; charset=utf-8";
    mime_types_[".js"] = "application/javascript; charset=utf-8";
    mime_types_[".json"] = "application/json; charset=utf-8";
    mime_types_[".jpg"] = "image/jpeg";
    mime_types_[".jpeg"] = "image/jpeg";
    mime_types_[".png"] = "image/png";
    mime_types_[".gif"] = "image/gif";
    mime_types_[".ico"] = "image/x-icon";
    mime_types_[".txt"] = "text/plain; charset=utf-8";
    mime_types_[".pdf"] = "application/pdf";
    mime_types_[".svg"] = "image/svg+xml";
    mime_types_[".woff"] = "font/woff";
    mime_types_[".woff2"] = "font/woff2";
}

http::HttpResponse StaticHandler::handle(const http::HttpRequest& request) {
    http::HttpResponse response;
    
    std::string file_path = request.uri;
    if (file_path.empty() || file_path == "/") {
        file_path = "/index.html";
    }
    
    file_path = normalize_path(file_path);
    
    if (!is_safe_path(file_path)) {
        return load_error_page(403, "Forbidden");
    }
    
    const std::string full_path = web_root_ + file_path;
    
    LOG_DEBUG("Trying to serve file: " + full_path);
    
    std::ifstream file(full_path, std::ios::binary);
    if (!file.is_open()) {
        LOG_WARNING("File not found: " + full_path);
        return load_error_page(404, "Not Found");
    }
    
    std::stringstream buffer;
    buffer << file.rdbuf();
    response.body = buffer.str();
    
    const std::string content_type = get_content_type(file_path);
    response.headers["Content-Type"] = content_type;
    
    LOG_INFO("Served file: " + file_path + " (" + content_type + ")");
    
    return response;
}

http::HttpResponse StaticHandler::load_error_page(int code, const std::string& status_text) {
    http::HttpResponse response;
    response.status_code = code;
    response.status_text = status_text;
    response.headers["Content-Type"] = "text/html; charset=utf-8";
    
    std::string error_file = "/error-" + std::to_string(code) + ".html";
    std::ifstream file(web_root_ + error_file, std::ios::binary);
    
    if (file.is_open()) {
        std::stringstream buffer;
        buffer << file.rdbuf();
        response.body = buffer.str();
        LOG_INFO("Served error page: " + error_file);
    } else {
        response.body = "<h1>" + std::to_string(code) + " " + status_text + "</h1>";
        LOG_WARNING("Error page not found: " + error_file);
    }
    
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

}