#include "core/server.hpp"
#include "core/config.hpp"
#include "utils/logger.hpp"
#include "http/static_handler.hpp"
#include "http/http.hpp"
#include "nlohmann/json.hpp"
#include <iostream>
#include <stdexcept>
#include <cstdlib>

using json = nlohmann::json;

int main() {
    try {
        const auto config = https_server::Config::load();
        
        https_server::Logger::instance().set_level(config.log_level);
        LOG_INFO("Configuration loaded successfully");
        
        https_server::Server server(config);
        auto& router = server.get_router();

        https_server::StaticHandler static_handler(config.web_root);

        router.add_route("GET", "/", [&static_handler, &config](const https_server::http::HttpRequest& req) {
            https_server::http::HttpRequest index_req = req;
            index_req.uri = "/index.html";
            auto response = static_handler.handle(index_req);
            response.security_config = &config.security;
            return response;
        });

        router.add_route("GET", "/about", [&config](const https_server::http::HttpRequest&) {
            https_server::http::HttpResponse response;
            response.security_config = &config.security;
            response.body = R"(
<!DOCTYPE html>
<html lang="pt-BR">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>Sobre - HTTPS Server</title>
    <link rel="stylesheet" href="/style.css">
</head>
<body>
    <div class="test-container">
        <div class="card">
            <h1>🚀 Sobre o HTTPS Server</h1>
            <p>Servidor HTTPS desenvolvido com <strong>C++17</strong></p>
            <p>Assembly customizado com <code>AES-NI</code> e <code>SHA-256 AVX</code></p>
            <p>Provider OpenSSL customizado para máxima performance</p>
            <p>Arquitetura multi-threaded com buffer de alta performance</p>
            <a href="/" class="btn">← Voltar ao Início</a>
        </div>
    </div>
</body>
</html>
            )";
            response.headers["Content-Type"] = "text/html; charset=utf-8";
            return response;
        });

        router.add_route("POST", "/api/echo", [&config](const https_server::http::HttpRequest& req) {
            https_server::http::HttpResponse response;
            response.security_config = &config.security;
            response.headers["Content-Type"] = "application/json; charset=utf-8";
            
            try {
                if (req.body.empty()) {
                    json error_response;
                    error_response["error"] = "Empty request body";
                    error_response["status"] = "error";
                    response.body = error_response.dump(2);
                    response.status_code = 400;
                    response.status_text = "Bad Request";
                } else {
                    json request_json = json::parse(req.body);
                    
                    json response_json = request_json;
                    response_json["received"] = true;
                    response_json["timestamp"] = std::time(nullptr);
                    response_json["server"] = "HTTPS Server v1.0";
                    response_json["status"] = "success";
                    
                    response.body = response_json.dump(2);
                }
            } catch (const json::exception& e) {
                json error_response;
                error_response["error"] = "Invalid JSON";
                error_response["details"] = e.what();
                error_response["status"] = "error";
                response.body = error_response.dump(2);
                response.status_code = 400;
                response.status_text = "Bad Request";
            }
            
            return response;
        });

        router.add_route("GET", "/style.css", [&static_handler, &config](const https_server::http::HttpRequest& req) {
            auto response = static_handler.handle(req);
            response.security_config = &config.security;
            return response;
        });

        router.add_route("GET", "/test.html", [&static_handler, &config](const https_server::http::HttpRequest& req) {
            auto response = static_handler.handle(req);
            response.security_config = &config.security;
            return response;
        });

        router.add_route("GET", "/static/*", [&static_handler, &config](const https_server::http::HttpRequest& req) {
            https_server::http::HttpRequest static_req = req;
            static_req.uri = req.uri.substr(7);
            auto response = static_handler.handle(static_req);
            response.security_config = &config.security;
            return response;
        });

        server.run();

    } catch (const std::exception& e) {
        LOG_ERROR("Fatal error: " + std::string(e.what()));
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}