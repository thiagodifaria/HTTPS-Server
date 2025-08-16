#include "http/static_handler.hpp"
#include "utils/logger.hpp"
#include <fstream>
#include <sstream>
#include <filesystem>

namespace https_server {

StaticHandler::StaticHandler(const std::string& web_root) 
    : web_root_(web_root) {
    init_mime_types();
}

void StaticHandler::init_mime_types() {
    mime_types_[".html"] = "text/html; charset=utf-8";
    mime_types_[".htm"] = "text/html; charset=utf-8";
    mime_types_[".css"] = "text/css; charset=utf-8";
    mime_types_[".js"] = "application/javascript; charset=utf-8";
    mime_types_[".json"] = "application/json; charset=utf-8";
    mime_types_[".jpg"] = "image/jpeg";
    mime_types_[".jpeg"] = "image/jpeg";
    mime_types_[".png"] = "image/png";
    mime_types_[".gif"] = "image/gif";
    mime_types_[".ico"] = "image/x-icon";
    mime_types_[".txt"] = "text/plain; charset=utf-8";
    mime_types_[".pdf"] = "application/pdf";
    mime_types_[".svg"] = "image/svg+xml";
    mime_types_[".woff"] = "font/woff";
    mime_types_[".woff2"] = "font/woff2";
}

http::HttpResponse StaticHandler::handle(const http::HttpRequest& request) {
    http::HttpResponse response;
    
    std::string file_path = request.uri;
    if (file_path.empty() || file_path == "/") {
        file_path = "/index.html";
    }
    
    file_path = normalize_path(file_path);
    
    if (!is_safe_path(file_path)) {
        response.status_code = 403;
        response.status_text = "Forbidden";
        response.body = create_error_page("403", "Acesso Negado", "Você não tem permissão para acessar este arquivo.");
        response.headers["Content-Type"] = "text/html; charset=utf-8";
        return response;
    }
    
    const std::string full_path = web_root_ + file_path;
    
    std::ifstream file(full_path, std::ios::binary);
    if (!file.is_open()) {
        response.status_code = 404;
        response.status_text = "Not Found";
        response.body = create_error_page("404", "Arquivo Não Encontrado", "O arquivo solicitado não foi encontrado no servidor.");
        response.headers["Content-Type"] = "text/html; charset=utf-8";
        return response;
    }
    
    std::stringstream buffer;
    buffer << file.rdbuf();
    response.body = buffer.str();
    
    const std::string content_type = get_content_type(file_path);
    response.headers["Content-Type"] = content_type;
    
    LOG_INFO("Served static file: " + file_path);
    
    return response;
}

std::string StaticHandler::get_content_type(const std::string& file_path) const {
    const size_t dot_pos = file_path.find_last_of('.');
    if (dot_pos == std::string::npos) {
        return "application/octet-stream";
    }
    
    const std::string ext = file_path.substr(dot_pos);
    const auto it = mime_types_.find(ext);
    return (it != mime_types_.end()) ? it->second : "application/octet-stream";
}

bool StaticHandler::is_safe_path(const std::string& requested_path) const {
    return requested_path.find("..") == std::string::npos;
}

std::string StaticHandler::normalize_path(const std::string& path) const {
    std::string normalized = path;
    for (char& c : normalized) {
        if (c == '\\') c = '/';
    }
    return normalized;
}

std::string StaticHandler::create_error_page(const std::string& code, const std::string& title, const std::string& message) const {
    return R"(
<!DOCTYPE html>
<html lang="pt-BR">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>)" + code + " " + title + R"( - HTTPS Server</title>
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
            font-size: 3rem;
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
        <h1>)" + code + R"(</h1>
        <h2>)" + title + R"(</h2>
        <p>)" + message + R"(</p>
        <a href="/">← Voltar à página inicial</a>
    </div>
</body>
</html>
    )";
}

}