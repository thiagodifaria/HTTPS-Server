#ifndef HTTPS_SERVER_ROUTER_HPP
#define HTTPS_SERVER_ROUTER_HPP

#include "http.hpp"
#include <functional>
#include <map>
#include <string>

namespace https_server {

using HttpHandler = std::function<http::HttpResponse(const http::HttpRequest&)>;

class Router {
public:
    void add_route(const std::string& method, const std::string& uri, HttpHandler handler) {
        routes_[uri][method] = std::move(handler);
    }

    http::HttpResponse route_request(const http::HttpRequest& request) const {
        auto uri_it = routes_.find(request.uri);
        if (uri_it != routes_.end()) {
            auto method_it = uri_it->second.find(request.method);
            if (method_it != uri_it->second.end()) {
                return method_it->second(request);
            }
        }
        
        // Se nenhuma rota for encontrada, retorna 404 Not Found
        http::HttpResponse response;
        response.status_code = 404;
        response.status_text = "Not Found";
        response.body = "<h1>404 Not Found</h1>";
        return response;
    }

private:
    std::map<std::string, std::map<std::string, HttpHandler>> routes_;
};

} // namespace https_server

#endif // HTTPS_SERVER_ROUTER_HPP