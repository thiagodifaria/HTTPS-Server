#ifndef HTTPS_SERVER_ROUTER_HPP
#define HTTPS_SERVER_ROUTER_HPP

#include "http.hpp"
#include <functional>
#include <map>
#include <string>
#include <vector>

namespace https_server {

using HttpHandler = std::function<http::HttpResponse(const http::HttpRequest&)>;

struct Route {
    std::string pattern;
    std::string method;
    HttpHandler handler;
    bool is_wildcard;
};

class Router {
public:
    void add_route(const std::string& method, const std::string& pattern, HttpHandler handler) {
        Route route;
        route.method = method;
        route.pattern = pattern;
        route.handler = std::move(handler);
        route.is_wildcard = pattern.find('*') != std::string::npos;
        
        routes_.push_back(route);
    }

    http::HttpResponse route_request(const http::HttpRequest& request) const {
        for (const auto& route : routes_) {
            if (route.method != request.method) {
                continue;
            }
            
            if (matches_pattern(route.pattern, request.uri)) {
                return route.handler(request);
            }
        }
        
        http::HttpResponse response;
        response.status_code = 404;
        response.status_text = "Not Found";
        response.body = "<h1>404 Not Found</h1>";
        response.headers["Content-Type"] = "text/html; charset=utf-8";
        return response;
    }

private:
    std::vector<Route> routes_;
    
    bool matches_pattern(const std::string& pattern, const std::string& uri) const {
        if (pattern.find('*') == std::string::npos) {
            return pattern == uri;
        }
        
        if (pattern.back() == '*' && pattern.size() > 1 && pattern[pattern.size()-2] == '/') {
            const std::string prefix = pattern.substr(0, pattern.size() - 1);
            return uri.substr(0, prefix.size()) == prefix;
        }
        
        return pattern == uri;
    }
};

}

#endif