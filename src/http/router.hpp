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
        response.body = create_404_page();
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
    
    std::string create_404_page() const {
        return R"(
<!DOCTYPE html>
<html lang="pt-BR">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>404 Not Found - HTTPS Server</title>
    <style>
        body { 
            font-family: 'Segoe UI', sans-serif; 
            background: linear-gradient(135deg, #667eea 0%, #764ba2 100%);
            min-height: 100vh;
            margin: 0;
            display: flex;
            align-items: center;
            justify-content: center;
        }
        .container {
            background: rgba(255,255,255,0.95);
            padding: 40px;
            border-radius: 20px;
            text-align: center;
            box-shadow: 0 20px 40px rgba(0,0,0,0.1);
            max-width: 500px;
        }
        h1 { 
            color: #e74c3c; 
            font-size: 4rem;
            margin-bottom: 10px;
        }
        h2 {
            color: #333;
            margin-bottom: 20px;
        }
        p { 
            color: #666; 
            margin-bottom: 30px;
            font-size: 1.1rem;
        }
        a { 
            background: linear-gradient(45deg, #667eea, #764ba2);
            color: white;
            padding: 12px 24px;
            text-decoration: none;
            border-radius: 8px;
            font-weight: 600;
            transition: transform 0.3s ease;
            display: inline-block;
        }
        a:hover { 
            transform: translateY(-2px);
        }
    </style>
</head>
<body>
    <div class="container">
        <h1>404</h1>
        <h2>Página Não Encontrada</h2>
        <p>A rota solicitada não foi encontrada no servidor.</p>
        <a href="/">← Voltar à página inicial</a>
    </div>
</body>
</html>
        )";
    }
};

}

#endif