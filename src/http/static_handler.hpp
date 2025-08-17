#ifndef HTTPS_SERVER_STATIC_HANDLER_HPP
#define HTTPS_SERVER_STATIC_HANDLER_HPP

#include "http/http.hpp"
#include <string>
#include <map>

namespace https_server {

class StaticHandler {
public:
    explicit StaticHandler(const std::string& web_root);
    
    http::HttpResponse handle(const http::HttpRequest& request);

private:
    std::string web_root_;
    std::map<std::string, std::string> mime_types_;
    
    void init_mime_types();
    std::string get_content_type(const std::string& file_path) const;
    bool is_safe_path(const std::string& requested_path) const;
    std::string normalize_path(const std::string& path) const;
    http::HttpResponse load_error_page(int code, const std::string& status_text);
};

}

#endif