#include "core/server.hpp"
#include "http/http.hpp"
#include <iostream>
#include <stdexcept>
#include <cstdlib>

int main() {
    constexpr std::uint16_t port = 8443;

    try {
        https_server::Server server(port);

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

        server.run();

    } catch (const std::exception& e) {
        std::cerr << "Fatal Error: " << e.what() << std::endl;
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}