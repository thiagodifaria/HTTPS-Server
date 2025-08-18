#ifdef _WIN32
#define NOMINMAX
#define _CRT_SECURE_NO_WARNINGS
#endif

#include "core/server.hpp"
#include "utils/logger.hpp"
#include "utils/buffer.hpp"
#include "utils/fast_memory.hpp"
#include "utils/http_accelerated.hpp"
#include "http/http.hpp"
#include <iostream>
#include <stdexcept>
#include <system_error>
#include <thread>
#include <string>
#include <sstream>
#include <algorithm>
#include <openssl/ssl.h>
#include <openssl/err.h>
#include <openssl/provider.h>

#ifdef _WIN32
#include <ws2tcpip.h>
#include <signal.h>
#pragma comment(lib, "ws2_32.lib")
static BOOL WINAPI console_ctrl_handler(DWORD ctrl_type);
static https_server::Server* g_server_instance = nullptr;
#else
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <signal.h>
static void signal_handler(int signum);
static https_server::Server* g_server_instance = nullptr;
#endif

namespace https_server {

void log_openssl_errors();
#ifdef _WIN32
void close_socket(SOCKET s);
#else
void close_socket(SOCKET s);
#endif
http::HttpRequest parse_request(const Buffer& buffer);

extern "C" int OSSL_provider_init(const OSSL_CORE_HANDLE *handle, 
                                  const OSSL_DISPATCH *in, 
                                  const OSSL_DISPATCH **out, 
                                  void **provctx);

Server::Server(const ServerConfig& config) 
    : config_(config),
      server_socket_(static_cast<SOCKET>(-1)), 
      pool_(config.threads == 0 ? std::thread::hardware_concurrency() : config.threads),
      ssl_ctx_(nullptr),
      default_provider_(nullptr),
      custom_provider_(nullptr),
      running_(true)
{
#ifdef _WIN32
    if (WSAStartup(MAKEWORD(2, 2), &wsa_data_) != 0) {
        throw std::runtime_error("WSAStartup failed");
    }
#endif
    
    LOG_INFO("Starting HTTPS server on port " + std::to_string(config_.port));
    
    if (fast_memory::MemoryOps::instance().has_avx2()) {
        LOG_INFO("AVX2 memory optimizations enabled");
    } else {
        LOG_INFO("Using standard memory operations (AVX2 not available)");
    }
    
    if (http_accelerated::HttpOps::instance().has_avx2()) {
        LOG_INFO("HTTP parsing optimizations enabled");
    } else {
        LOG_INFO("Using standard HTTP parsing");
    }
    
#ifdef HAS_CRYPTO_ADVANCED
    LOG_INFO("Advanced crypto algorithms available (ChaCha20, Blake3, X25519)");
#else
    LOG_INFO("Using standard crypto algorithms");
#endif
    
    init_openssl();
    setup_providers();
    load_openssl_config();
    create_ssl_context();
    setup_signal_handlers();
    
    event_loop_ = std::make_unique<EventLoop>();
}

Server::~Server() {
    shutdown();
    
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

    if (server_socket_ != static_cast<SOCKET>(-1)) {
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
            LOG_INFO("Custom crypto provider loaded (AES/SHA-256/ChaCha20/Blake3/X25519)");
        }
    }
    
    if (!custom_provider_) {
#ifdef _WIN32
        custom_provider_ = OSSL_PROVIDER_load(NULL, ".\\aes_provider.dll");
#else
        custom_provider_ = OSSL_PROVIDER_load(NULL, "./libaes_provider.so");
#endif
        if (custom_provider_) {
            LOG_INFO("External crypto provider loaded");
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
    
    if (!config_.client_ca_file.empty()) {
        LOG_INFO("Configuring mutual TLS authentication");
        
        if (SSL_CTX_load_verify_locations(ssl_ctx_, config_.client_ca_file.c_str(), nullptr) != 1) {
            log_openssl_errors();
            throw std::runtime_error("Failed to load client CA file: " + config_.client_ca_file);
        }
        
        SSL_CTX_set_verify(ssl_ctx_, 
                          SSL_VERIFY_PEER | SSL_VERIFY_FAIL_IF_NO_PEER_CERT, 
                          nullptr);
        
        LOG_INFO("Mutual TLS authentication enabled");
    }
    
    LOG_INFO("SSL context created with cert: " + config_.cert_file + ", key: " + config_.key_file);
}

void Server::setup_signal_handlers() {
    g_server_instance = this;
    
#ifdef _WIN32
    SetConsoleCtrlHandler(console_ctrl_handler, TRUE);
#else
    struct sigaction sa;
    sa.sa_handler = signal_handler;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = 0;
    
    sigaction(SIGINT, &sa, nullptr);
    sigaction(SIGTERM, &sa, nullptr);
    sigaction(SIGHUP, &sa, nullptr);
#endif
    
    LOG_DEBUG("Signal handlers configured");
}

void Server::handle_shutdown_signal() {
    LOG_INFO("Received shutdown signal, initiating graceful shutdown");
    running_ = false;
}

void Server::handle_reload_signal() {
    LOG_INFO("Received reload signal, reloading SSL certificates");
    try {
        reload_ssl_context();
        LOG_INFO("SSL certificates reloaded successfully");
    } catch (const std::exception& e) {
        LOG_ERROR("Failed to reload SSL certificates: " + std::string(e.what()));
    }
}

void Server::reload_ssl_context() {
    std::lock_guard<std::mutex> lock(ssl_context_mutex_);
    
    SSL_CTX* new_ctx = SSL_CTX_new(TLS_server_method());
    if (!new_ctx) {
        throw std::runtime_error("Failed to create new SSL context");
    }
    
    if (SSL_CTX_use_certificate_file(new_ctx, config_.cert_file.c_str(), SSL_FILETYPE_PEM) <= 0) {
        SSL_CTX_free(new_ctx);
        throw std::runtime_error("Failed to load new certificate file");
    }
    
    if (SSL_CTX_use_PrivateKey_file(new_ctx, config_.key_file.c_str(), SSL_FILETYPE_PEM) <= 0) {
        SSL_CTX_free(new_ctx);
        throw std::runtime_error("Failed to load new private key file");
    }
    
    if (!config_.client_ca_file.empty()) {
        if (SSL_CTX_load_verify_locations(new_ctx, config_.client_ca_file.c_str(), nullptr) != 1) {
            SSL_CTX_free(new_ctx);
            throw std::runtime_error("Failed to load new client CA file");
        }
        SSL_CTX_set_verify(new_ctx, SSL_VERIFY_PEER | SSL_VERIFY_FAIL_IF_NO_PEER_CERT, nullptr);
    }
    
    SSL_CTX* old_ctx = ssl_ctx_;
    ssl_ctx_ = new_ctx;
    SSL_CTX_free(old_ctx);
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

void Server::handle_new_connection(SOCKET server_socket) {
    sockaddr_in client_addr{};
    socklen_t client_len = sizeof(client_addr);
    const SOCKET client_socket = accept(server_socket, reinterpret_cast<sockaddr*>(&client_addr), &client_len);

#ifdef _WIN32
    if (client_socket == INVALID_SOCKET) {
#else
    if (client_socket < 0) {
#endif
        return;
    }

    event_loop_->add_socket(client_socket, [this](SOCKET sock) {
        handle_client_data(sock);
    });
}

void Server::handle_client_data(SOCKET client_socket) {
    pool_.enqueue([this, client_socket] {
        handle_connection(client_socket);
    });
}

void Server::handle_connection(SOCKET client_socket) {
    std::lock_guard<std::mutex> lock(ssl_context_mutex_);
    
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
        Buffer buffer;
        
        while (true) {
            const int bytes_read = SSL_read(ssl, buffer.write_ptr(), 
                                          static_cast<int>(buffer.writable_bytes()));
            if (bytes_read <= 0) break;
            
            buffer.has_written(static_cast<size_t>(bytes_read));
            
            size_t header_end_pos;
            if (http_accelerated::HttpOps::instance().find_header_end(
                buffer.readable_view().data(), buffer.readable_view().size(), &header_end_pos)) {
                break;
            }
            
            buffer.ensure_capacity(4096);
        }

        if (buffer.readable_bytes() > 0) {
            const http::HttpRequest request = parse_request(buffer);
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

    event_loop_->add_socket(server_socket_, [this](SOCKET sock) {
        handle_new_connection(sock);
    });
    
    while (running_) {
        event_loop_->run_once(100);
    }
    
    LOG_INFO("Main event loop exited");
}

void Server::shutdown() {
    if (running_) {
        running_ = false;
        pool_.shutdown();
        LOG_INFO("Server shutdown completed");
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

http::HttpRequest parse_request(const Buffer& buffer) {
    http::HttpRequest request;
    const auto view = buffer.readable_view();
    
    auto parse_result = http_accelerated::HttpOps::instance().parse_method_uri(
        view.data(), view.size());
    
    if (parse_result.valid) {
        request.method = std::string(view.data(), parse_result.method_len);
        request.uri = std::string(view.data() + parse_result.uri_start, parse_result.uri_len);
        request.http_version = "HTTP/1.1";
    }
    
    std::string raw_request(view);
    std::istringstream request_stream(raw_request);
    std::string line;

    if (parse_result.valid) {
        std::getline(request_stream, line);
    } else {
        if (std::getline(request_stream, line)) {
            if (!line.empty() && line.back() == '\r') {
                line.pop_back();
            }
            std::istringstream request_line_stream(line);
            request_line_stream >> request.method >> request.uri >> request.http_version;
        }
    }

    size_t content_length = 0;

    while (std::getline(request_stream, line) && !line.empty() && line != "\r") {
        if (!line.empty() && line.back() == '\r') {
            line.pop_back();
        }
        const size_t colon_pos = line.find(':');
        if (colon_pos != std::string::npos) {
            const std::string key = line.substr(0, colon_pos);
            const std::string value = line.substr(colon_pos + 2);
            request.headers[key] = value;
            
            if (key == "Content-Length") {
                content_length = std::stoul(value);
            }
        }
    }

    if (content_length > 0) {
        const char* headers_end = static_cast<const char*>(
            fast_memory::memchr(raw_request.data(), '\r', raw_request.size())
        );
        
        if (headers_end) {
            const char* body_start = static_cast<const char*>(
                fast_memory::memchr(headers_end, '\n', raw_request.size() - (headers_end - raw_request.data()))
            );
            
            if (body_start) {
                body_start++;
                const size_t body_offset = body_start - raw_request.data();
                if (body_offset < raw_request.size()) {
                    const size_t body_available = raw_request.size() - body_offset;
                    const size_t body_size = (std::min)(content_length, body_available);
                    request.body = raw_request.substr(body_offset, body_size);
                }
            }
        }
    }

    return request;
}

} // namespace https_server

#ifdef _WIN32
BOOL WINAPI console_ctrl_handler(DWORD ctrl_type) {
    if (g_server_instance) {
        switch (ctrl_type) {
            case CTRL_C_EVENT:
            case CTRL_BREAK_EVENT:
            case CTRL_CLOSE_EVENT:
                g_server_instance->handle_shutdown_signal();
                return TRUE;
        }
    }
    return FALSE;
}
#else
void signal_handler(int signum) {
    if (g_server_instance) {
        switch (signum) {
            case SIGINT:
            case SIGTERM:
                g_server_instance->handle_shutdown_signal();
                break;
            case SIGHUP:
                g_server_instance->handle_reload_signal();
                break;
        }
    }
}
#endif