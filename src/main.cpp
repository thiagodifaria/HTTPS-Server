#include "core/server.hpp"
#include "core/config.hpp"
#include "utils/logger.hpp"
#include "http/static_handler.hpp"
#include "http/http.hpp"
#include <iostream>
#include <stdexcept>
#include <cstdlib>

int main() {
    try {
        const auto config = https_server::Config::load();
        
        https_server::Logger::instance().set_level(config.log_level);
        LOG_INFO("Configuration loaded successfully");
        
        https_server::Server server(config);
        auto& router = server.get_router();

        router.add_route("GET", "/", [](const https_server::http::HttpRequest& req) {
            https_server::http::HttpResponse response;
            response.body = "<h1>Página Principal</h1><p>Bem-vindo ao servidor HTTPS com roteamento!</p>";
            return response;
        });

        router.add_route("GET", "/about", [](const https_server::http::HttpRequest& req) {
            https_server::http::HttpResponse response;
            response.body = "<h1>Sobre</h1><p>Servidor desenvolvido com C++, Assembly e um provedor OpenSSL customizado.</p>";
            return response;
        });

        https_server::StaticHandler static_handler(config.web_root);
        router.add_route("GET", "/static/*", [&static_handler](const https_server::http::HttpRequest& req) {
            return static_handler.handle(req);
        });

        server.run();

    } catch (const std::exception& e) {
        LOG_ERROR("Fatal error: " + std::string(e.what()));
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}