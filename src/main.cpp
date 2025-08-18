#include "core/server.hpp"
#include "core/config.hpp"
#include "utils/logger.hpp"
#include "utils/validation_engine.hpp"
#include "utils/network_operations.hpp"
#include "utils/benchmark_utils.hpp"
#include "http/static_handler.hpp"
#include "http/http.hpp"
#include "nlohmann/json.hpp"
#include <iostream>
#include <stdexcept>
#include <cstdlib>
#include <iomanip>
#include <sstream>

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

        router.add_route("GET", "/about", [&static_handler, &config](const https_server::http::HttpRequest& req) {
            https_server::http::HttpRequest about_req = req;
            about_req.uri = "/about.html";
            auto response = static_handler.handle(about_req);
            response.security_config = &config.security;
            return response;
        });

        router.add_route("GET", "/bench", [&static_handler, &config](const https_server::http::HttpRequest& req) {
            https_server::http::HttpRequest bench_req = req;
            bench_req.uri = "/bench.html";
            auto response = static_handler.handle(bench_req);
            response.security_config = &config.security;
            return response;
        });

        router.add_route("GET", "/api/benchmark", [&config](const https_server::http::HttpRequest& req) {
            std::string request_id = https_server::Logger::instance().generate_request_id();
            
            https_server::http::HttpResponse response;
            response.security_config = &config.security;
            response.headers["Content-Type"] = "application/json; charset=utf-8";
            response.headers["X-Request-ID"] = request_id;
            
            try {
                auto aes_result = https_server::benchmark::run_aes_benchmark();
                auto sha_result = https_server::benchmark::run_sha256_benchmark();
                auto p256_result = https_server::benchmark::run_p256_benchmark();

                json response_json;
                response_json["status"] = "success";
                response_json["request_id"] = request_id;
                response_json["timestamp"] = std::time(nullptr);

                std::stringstream aes_throughput, sha_throughput, aes_time, sha_time;
                aes_throughput << std::fixed << std::setprecision(2) << aes_result.throughput_gb_s << " GB/s";
                sha_throughput << std::fixed << std::setprecision(2) << sha_result.throughput_gb_s << " GB/s";
                aes_time << std::fixed << std::setprecision(3) << aes_result.duration_seconds << "s";
                sha_time << std::fixed << std::setprecision(3) << sha_result.duration_seconds << "s";

                response_json["aes_ni"]["throughput"] = aes_throughput.str();
                response_json["aes_ni"]["time"] = aes_time.str();
                response_json["aes_ni"]["blocks"] = aes_result.operations;

                response_json["sha256"]["throughput"] = sha_throughput.str();
                response_json["sha256"]["time"] = sha_time.str();
                response_json["sha256"]["blocks"] = sha_result.operations;

                response_json["p256"]["field_ops"] = static_cast<int>(p256_result.field_ops_per_sec);
                response_json["p256"]["point_ops"] = static_cast<int>(p256_result.point_ops_per_sec);
                response_json["p256"]["ecdh_est"] = static_cast<int>(p256_result.estimated_ecdh_per_sec);

                response.body = response_json.dump(2);
            } catch (const std::exception& e) {
                json error_response;
                error_response["error"] = "Benchmark execution failed";
                error_response["details"] = e.what();
                error_response["status"] = "error";
                error_response["request_id"] = request_id;
                response.body = error_response.dump(2);
                response.status_code = 500;
                response.status_text = "Internal Server Error";
            }
            
            return response;
        });

        router.add_route("POST", "/api/echo", [&config](const https_server::http::HttpRequest& req) {
            std::string request_id = https_server::Logger::instance().generate_request_id();
            
            https_server::http::HttpResponse response;
            response.security_config = &config.security;
            response.headers["Content-Type"] = "application/json; charset=utf-8";
            response.headers["X-Request-ID"] = request_id;
            
            try {
                if (req.body.empty()) {
                    json error_response;
                    error_response["error"] = "Empty request body";
                    error_response["status"] = "error";
                    error_response["request_id"] = request_id;
                    response.body = error_response.dump(2);
                    response.status_code = 400;
                    response.status_text = "Bad Request";
                } else {
                    auto validation_result = https_server::validation::ValidationOps::instance()
                        .json_validate_fast(req.body.c_str(), req.body.size());
                    
                    if (validation_result != https_server::validation::ValidationResult::VALID) {
                        json error_response;
                        error_response["status"] = "error";
                        error_response["request_id"] = request_id;
                        
                        switch (validation_result) {
                            case https_server::validation::ValidationResult::INVALID_JSON:
                                error_response["error"] = "Invalid JSON format";
                                break;
                            case https_server::validation::ValidationResult::INVALID_UTF8:
                                error_response["error"] = "Invalid UTF-8 encoding";
                                break;
                            case https_server::validation::ValidationResult::UNSAFE_CHARS:
                                error_response["error"] = "Unsafe characters detected";
                                break;
                            default:
                                error_response["error"] = "Validation failed";
                                break;
                        }
                        
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
                        response_json["request_id"] = request_id;
                        
                        if (request_json.contains("encode_data")) {
                            std::string data = request_json["encode_data"];
                            response_json["base64_encoded"] = https_server::network_ops::NetworkOps::instance()
                                .encode_base64(data);
                            response_json["hex_encoded"] = https_server::network_ops::NetworkOps::instance()
                                .encode_hex(data);
                        }
                        
                        response.body = response_json.dump(2);
                    }
                }
            } catch (const json::exception& e) {
                json error_response;
                error_response["error"] = "JSON parsing failed";
                error_response["details"] = e.what();
                error_response["status"] = "error";
                error_response["request_id"] = request_id;
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

        router.add_route("GET", "/*", [&static_handler, &config](const https_server::http::HttpRequest& req) {
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