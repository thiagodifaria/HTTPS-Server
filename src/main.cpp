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

        router.add_route("GET", "/", [&config](const https_server::http::HttpRequest&) {
            https_server::http::HttpResponse response;
            response.security_config = &config.security;
            response.body = "<h1>Página Principal</h1><p>Bem-vindo ao servidor HTTPS com roteamento!</p>";
            return response;
        });

        router.add_route("GET", "/about", [&config](const https_server::http::HttpRequest&) {
            https_server::http::HttpResponse response;
            response.security_config = &config.security;
            response.body = "<h1>Sobre</h1><p>Servidor desenvolvido com C++, Assembly e um provedor OpenSSL customizado.</p>";
            return response;
        });

        router.add_route("POST", "/api/echo", [&config](const https_server::http::HttpRequest& req) {
            https_server::http::HttpResponse response;
            response.security_config = &config.security;
            
            try {
                if (req.body.empty()) {
                    response.status_code = 400;
                    response.status_text = "Bad Request";
                    response.body = R"({"error": "Empty request body"})";
                } else {
                    json request_json = json::parse(req.body);
                    
                    json response_json = request_json;
                    response_json["received"] = true;
                    response_json["timestamp"] = std::time(nullptr);
                    response_json["server"] = "HTTPS Server v1.0";
                    
                    response.body = response_json.dump(2);
                    response.headers["Content-Type"] = "application/json; charset=utf-8";
                }
            } catch (const json::exception& e) {
                response.status_code = 400;
                response.status_text = "Bad Request";
                response.body = R"({"error": "Invalid JSON", "details": ")" + std::string(e.what()) + R"("})";
                response.headers["Content-Type"] = "application/json; charset=utf-8";
            }
            
            return response;
        });

        https_server::StaticHandler static_handler(config.web_root);
        router.add_route("GET", "/static/*", [&static_handler, &config](const https_server::http::HttpRequest& req) {
            auto response = static_handler.handle(req);
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