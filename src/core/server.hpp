#ifndef HTTPS_SERVER_SERVER_HPP
#define HTTPS_SERVER_SERVER_HPP

#include "core/thread_pool.hpp"
#include "core/config.hpp"
#include "http/router.hpp"
#include <cstdint>

#ifdef _WIN32
#include <winsock2.h>
#else
using SOCKET = int;
#endif

struct ssl_ctx_st;
using SSL_CTX = struct ssl_ctx_st;

struct ossl_provider_st;
using OSSL_PROVIDER = struct ossl_provider_st;

namespace https_server {

class Server {
public:
    explicit Server(const ServerConfig& config);
    ~Server();

    Server(const Server&) = delete;
    Server& operator=(const Server&) = delete;
    Server(Server&&) = delete;
    Server& operator=(Server&&) = delete;

    void run();
    Router& get_router() { return router_; }

private:
    void init_openssl();
    void cleanup_openssl();
    void setup_providers();
    void load_openssl_config();
    void create_ssl_context();
    void setup_socket();
    void handle_connection(SOCKET client_socket);

    const ServerConfig config_;
    SOCKET server_socket_;
    ThreadPool pool_;
    SSL_CTX* ssl_ctx_;
    Router router_;
    
    OSSL_PROVIDER* default_provider_;
    OSSL_PROVIDER* custom_provider_;

#ifdef _WIN32
    WSADATA wsa_data_;
#endif
};

} // namespace https_server

#endif // HTTPS_SERVER_SERVER_HPP