#ifndef HTTPS_SERVER_HTTP_HPP
#define HTTPS_SERVER_HTTP_HPP

#include <string>
#include <vector>
#include <map>
#include <sstream>

namespace https_server::http {

struct HttpRequest {
    std::string method;
    std::string uri;
    std::string http_version;
    std::map<std::string, std::string> headers;
};

struct HttpResponse {
    int status_code = 200;
    std::string status_text = "OK";
    std::map<std::string, std::string> headers;
    std::string body;

    std::string to_string() const {
        std::stringstream ss;
        ss << "HTTP/1.1 " << status_code << " " << status_text << "\r\n";
        
        if (headers.find("Content-Length") == headers.end()) {
             ss << "Content-Length: " << body.length() << "\r\n";
        }
        if (headers.find("Content-Type") == headers.end()) {
             ss << "Content-Type: text/html; charset=utf-8\r\n";
        }
        if (headers.find("Connection") == headers.end()) {
             ss << "Connection: close\r\n";
        }

        for (const auto& [key, value] : headers) {
            ss << key << ": " << value << "\r\n";
        }

        ss << "\r\n";
        ss << body;
        return ss.str();
    }
};

} // namespace https_server::http

#endif // HTTPS_SERVER_HTTP_HPP