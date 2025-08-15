#include "core/server.hpp"
#include "utils/logger.hpp"
#include "http/http.hpp"
#include <iostream>
#include <stdexcept>
#include <system_error>
#include <array>
#include <sstream>
#include <thread>
#include <string>
#include <openssl/ssl.h>
#include <openssl/err.h>
#include <openssl/provider.h>
#include <openssl/applink.c>

#ifdef _WIN32
#include <ws2tcpip.h>
#pragma comment(lib, "ws2_32.lib")
#else
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#endif

namespace https_server {

void log_openssl_errors();
#ifdef _WIN32
void close_socket(SOCKET s);
#else
void close_socket(SOCKET s);
#endif
http::HttpRequest parse_request(const std::string& raw_request);

extern "C" int OSSL_provider_init(const OSSL_CORE_HANDLE *handle, 
                                  const OSSL_DISPATCH *in, 
                                  const OSSL_DISPATCH **out, 
                                  void **provctx);

Server::Server(const ServerConfig& config) 
    : config_(config),
      server_socket_(-1), 
      pool_(config.threads == 0 ? std::thread::hardware_concurrency() : config.threads),
      ssl_ctx_(nullptr),
      default_provider_(nullptr),
      custom_provider_(nullptr)
{
#ifdef _WIN32
    if (WSAStartup(MAKEWORD(2, 2), &wsa_data_) != 0) {
        throw std::runtime_error("WSAStartup failed");
    }
#endif
    
    LOG_INFO("Starting HTTPS server on port " + std::to_string(config_.port));
    
    init_openssl();
    setup_providers();
    load_openssl_config();
    create_ssl_context();
}

Server::~Server() {
    if (custom_provider_) {
        OSSL_PROVIDER_unload(custom_provider_);
    }
    
    if (default_provider_) {
        OSSL_PROVIDER_unload(default_provider_);
    }
    
    if (ssl_ctx_) {
        SSL_CTX_free(ssl_ctx_);
    }
    
    cleanup_openssl();

    if (server_socket_ != -1) {
        close_socket(server_socket_);
    }
    
#ifdef _WIN32
    WSACleanup();
#endif
    
    LOG_INFO("Server stopped");
}

void Server::init_openssl() {
    SSL_load_error_strings();	
    OpenSSL_add_ssl_algorithms();
    LOG_DEBUG("OpenSSL initialized");
}

void Server::setup_providers() {
    default_provider_ = OSSL_PROVIDER_load(NULL, "default");
    if (!default_provider_) {
        log_openssl_errors();
        throw std::runtime_error("Failed to load default OpenSSL provider");
    }
    LOG_DEBUG("Default OpenSSL provider loaded");
    
    if (OSSL_PROVIDER_add_builtin(NULL, "aes_provider", OSSL_provider_init) == 1) {
        custom_provider_ = OSSL_PROVIDER_load(NULL, "aes_provider");
        if (custom_provider_) {
            LOG_INFO("Custom AES provider loaded");
        }
    }
    
    if (!custom_provider_) {
#ifdef _WIN32
        custom_provider_ = OSSL_PROVIDER_load(NULL, ".\\aes_provider.dll");
#else
        custom_provider_ = OSSL_PROVIDER_load(NULL, "./libaes_provider.so");
#endif
        if (custom_provider_) {
            LOG_INFO("External AES provider loaded");
        }
    }
}

void Server::load_openssl_config() {
    const char* root_dir = getenv("OPENSSL_ROOT_DIR");
    std::string config_path = root_dir ? 
        std::string(root_dir) + "\\bin\\cnf\\openssl.cnf" :
        "C:\\Program Files\\OpenSSL-Win64\\bin\\cnf\\openssl.cnf";
    
    if (FILE* file = fopen(config_path.c_str(), "r")) {
        fclose(file);
        if (OSSL_LIB_CTX_load_config(NULL, config_path.c_str()) == 1) {
            LOG_DEBUG("OpenSSL config loaded from: " + config_path);
        }
    }
}

void Server::cleanup_openssl() {
    EVP_cleanup();
}

void Server::create_ssl_context() {
    const SSL_METHOD *method = TLS_server_method();
    ssl_ctx_ = SSL_CTX_new(method);
    if (!ssl_ctx_) {
        log_openssl_errors();
        throw std::runtime_error("Unable to create SSL context");
    }

    if (SSL_CTX_use_certificate_file(ssl_ctx_, config_.cert_file.c_str(), SSL_FILETYPE_PEM) <= 0) {
        log_openssl_errors();
        throw std::runtime_error("Failed to load certificate file: " + config_.cert_file);
    }

    if (SSL_CTX_use_PrivateKey_file(ssl_ctx_, config_.key_file.c_str(), SSL_FILETYPE_PEM) <= 0) {
        log_openssl_errors();
        throw std::runtime_error("Failed to load private key file: " + config_.key_file);
    }
    
    LOG_INFO("SSL context created with cert: " + config_.cert_file + ", key: " + config_.key_file);
}

void Server::setup_socket() {
    server_socket_ = socket(AF_INET, SOCK_STREAM, 0);
#ifdef _WIN32
    if (server_socket_ == INVALID_SOCKET) {
        const int error = WSAGetLastError();
        throw std::runtime_error("Failed to create socket. Error: " + std::to_string(error));
    }
#else
    if (server_socket_ < 0) {
        throw std::system_error(errno, std::generic_category(), "Failed to create socket");
    }
#endif

    const int opt = 1;
#ifdef _WIN32
    if (setsockopt(server_socket_, SOL_SOCKET, SO_REUSEADDR, reinterpret_cast<const char*>(&opt), sizeof(opt)) == SOCKET_ERROR) {
        const int error = WSAGetLastError();
        close_socket(server_socket_);
        throw std::runtime_error("Failed to set socket options. Error: " + std::to_string(error));
    }
#else
    if (setsockopt(server_socket_, SOL_SOCKET, SO_REUSEADDR, reinterpret_cast<const char*>(&opt), sizeof(opt)) < 0) {
        throw std::system_error(errno, std::generic_category(), "Failed to set socket options");
    }
#endif

    sockaddr_in server_addr{};
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(config_.port);

#ifdef _WIN32
    if (bind(server_socket_, reinterpret_cast<const sockaddr*>(&server_addr), sizeof(server_addr)) == SOCKET_ERROR) {
        const int error = WSAGetLastError();
        close_socket(server_socket_);
        
        std::string error_msg = "Failed to bind socket. Error: " + std::to_string(error);
        if (error == WSAEADDRINUSE) {
            error_msg += " (Port " + std::to_string(config_.port) + " already in use)";
        }
        
        throw std::runtime_error(error_msg);
    }
#else
    if (bind(server_socket_, reinterpret_cast<const sockaddr*>(&server_addr), sizeof(server_addr)) < 0) {
        throw std::system_error(errno, std::generic_category(), "Failed to bind socket");
    }
#endif

#ifdef _WIN32
    if (listen(server_socket_, 32) == SOCKET_ERROR) {
        const int error = WSAGetLastError();
        close_socket(server_socket_);
        throw std::runtime_error("Failed to listen on socket. Error: " + std::to_string(error));
    }
#else
    if (listen(server_socket_, 32) < 0) {
        throw std::system_error(errno, std::generic_category(), "Failed to listen on socket");
    }
#endif
}

void Server::handle_connection(SOCKET client_socket) {
    SSL* ssl = SSL_new(ssl_ctx_);
    if (!ssl) {
        close_socket(client_socket);
        return;
    }
    
    SSL_set_fd(ssl, static_cast<int>(client_socket));

    if (SSL_accept(ssl) <= 0) {
        LOG_WARNING("SSL handshake failed");
        log_openssl_errors();
    } else {
        std::array<char, 4096> buffer{};
        const int bytes_read = SSL_read(ssl, buffer.data(), static_cast<int>(buffer.size()));

        if (bytes_read > 0) {
            const std::string raw_request(buffer.data(), bytes_read);
            const http::HttpRequest request = parse_request(raw_request);

            LOG_DEBUG("Request: " + request.method + " " + request.uri);

            const http::HttpResponse response = router_.route_request(request);
            const std::string response_str = response.to_string();
            SSL_write(ssl, response_str.c_str(), static_cast<int>(response_str.length()));
        }
    }
    
    SSL_shutdown(ssl);
    SSL_free(ssl);
    close_socket(client_socket);
}

void Server::run() {
    setup_socket();
    LOG_INFO("Server listening on port " + std::to_string(config_.port) + " with " + std::to_string(config_.threads == 0 ? std::thread::hardware_concurrency() : config_.threads) + " threads");

    while (true) {
        sockaddr_in client_addr{};
        socklen_t client_len = sizeof(client_addr);
        const SOCKET client_socket = accept(server_socket_, reinterpret_cast<sockaddr*>(&client_addr), &client_len);

#ifdef _WIN32
        if (client_socket == INVALID_SOCKET) {
#else
        if (client_socket < 0) {
#endif
            continue;
        }

        pool_.enqueue([this, client_socket] {
            handle_connection(client_socket);
        });
    }
}

void log_openssl_errors() {
    unsigned long err_code;
    while ((err_code = ERR_get_error())) {
        char err_msg[256];
        ERR_error_string_n(err_code, err_msg, sizeof(err_msg));
        LOG_ERROR("OpenSSL: " + std::string(err_msg));
    }
}

#ifdef _WIN32
void close_socket(const SOCKET s) { closesocket(s); }
#else
void close_socket(const SOCKET s) { close(s); }
#endif

http::HttpRequest parse_request(const std::string& raw_request) {
    http::HttpRequest request;
    std::stringstream request_stream(raw_request);
    std::string line;

    if (std::getline(request_stream, line)) {
        if (!line.empty() && line.back() == '\r') {
            line.pop_back();
        }
        std::stringstream request_line_stream(line);
        request_line_stream >> request.method >> request.uri >> request.http_version;
    }

    while (std::getline(request_stream, line) && !line.empty() && line != "\r") {
        if (!line.empty() && line.back() == '\r') {
            line.pop_back();
        }
        const size_t colon_pos = line.find(':');
        if (colon_pos != std::string::npos) {
            const std::string key = line.substr(0, colon_pos);
            const std::string value = line.substr(colon_pos + 2);
            request.headers[key] = value;
        }
    }
    return request;
}

} // namespace https_server