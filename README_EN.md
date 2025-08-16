# HTTPS Server - High-Performance C++ Web Server

Advanced HTTPS server implementation in C++17 featuring custom assembly-optimized cryptographic implementations and a high-performance OpenSSL provider. This project delivers a complete solution for high-throughput web applications, including hand-coded assembly cryptography, low-level buffer management, RESTful JSON APIs, and robust multi-threaded architecture.

## 🎯 Features

- ✅ **Custom cryptographic provider**: Assembly-optimized AES-NI and SHA-256 implementations
- ✅ **Extreme performance**: 3.51 GB/s AES throughput with multi-threaded architecture
- ✅ **Modern APIs**: RESTful JSON endpoints with robust HTTP router
- ✅ **High-performance buffer**: Zero-copy memory management with intelligent compaction
- ✅ **Production ready**: TLS 1.3, structured logging, optimized thread pool
- ✅ **Cross-platform**: Windows and Linux support with CMake build system
- ✅ **Modular architecture**: Clear separation of concerns and testable components

## 🗂️ Architecture

Modular architecture with clear separation of responsibilities:

```
src/
├── core/           # Infrastructure (server, config, thread pool)
├── crypto/         # Cryptographic engine and assembly optimizations
├── http/           # HTTP protocol, routing and static file serving
├── utils/          # Performance buffer, logging and utilities
└── main.cpp        # Entry point and route configuration
```

## 🔧 Technology Stack

### Core Technologies
- **C++17**: Modern C++ with advanced features
- **OpenSSL 3.0**: Cryptographic library and TLS implementation
- **CMake**: Cross-platform build system
- **NASM**: x86-64 assembler for performance optimizations

### Cryptography
- **AES-NI Assembly**: Hand-optimized implementation using Intel AES instructions
- **SHA-256 Assembly**: Cryptographic hashing with AVX registers
- **Custom OpenSSL Provider**: Native integration with OpenSSL stack

### Performance
- **Thread Pool**: Multi-threaded pool with automatic core detection
- **Zero-copy Buffer**: Custom buffer class with memory reuse
- **Structured Logging**: JSON structured logging system

### APIs and HTTP
- **nlohmann/json**: High-performance JSON processing
- **HTTP Router**: Routing system with wildcard support
- **Static File Server**: Optimized static file serving

## 📋 Prerequisites

- C++17+ compatible compiler (MSVC 2022, GCC 9+, Clang 10+)
- CMake 3.16+
- OpenSSL 3.0+
- NASM (for assembly compilation)

## 🚀 Quick Installation

### Windows (Recommended)

```bash
# Clone repository
git clone https://github.com/thiagodifaria/HTTPS-Server.git
cd HTTPS-Server

# Build with PowerShell
./build.ps1

# Run server
./build/Release/https_server.exe
```

### Linux/Unix

```bash
# Clone repository
git clone https://github.com/thiagodifaria/HTTPS-Server.git
cd HTTPS-Server

# Configure and build
mkdir build && cd build
cmake .. -DCMAKE_BUILD_TYPE=Release
make -j$(nproc)

# Run server
./https_server
```

## ⚙️ Configuration

### Environment Configuration

The server uses a `config.json` file for configuration:

```json
{
    "port": 8443,
    "threads": 0,
    "cert_file": "cert.pem",
    "key_file": "key.pem", 
    "web_root": "public",
    "log_level": "INFO"
}
```

### TLS Certificates

```bash
# Generate self-signed certificate for development
openssl req -x509 -newkey rsa:4096 -keyout key.pem -out cert.pem -days 365 -nodes
```

## 📊 API Usage

### Main Page

```bash
curl -k https://localhost:8443/
# Returns modern HTML interface with interactive design
```

### JSON Echo API

```bash
curl -k -X POST "https://localhost:8443/api/echo" \
     -H "Content-Type: application/json" \
     -d '{"message": "Hello World!", "timestamp": 1640995200}'
```

**Response:**
```json
{
  "message": "Hello World!",
  "timestamp": 1640995200,
  "received": true,
  "timestamp": 1234567890,
  "server": "HTTPS Server v1.0"
}
```

### Static File Serving

```bash
# Serve files from public/ directory
curl -k https://localhost:8443/static/test.html
curl -k https://localhost:8443/static/style.css
```

### Error Handling

```bash
# 404 handling
curl -k https://localhost:8443/nonexistent
# Returns styled 404 page

# JSON validation
curl -k -X POST "https://localhost:8443/api/echo" \
     -H "Content-Type: application/json" \
     -d 'invalid json'
# Returns JSON error response
```

## 🛠️ Main Endpoints

| Endpoint | Method | Description | Functionality |
|----------|--------|-------------|---------------|
| `/` | GET | Main page | Modern HTML interface |
| `/about` | GET | Server information | Technical details |
| `/api/echo` | POST | JSON echo API | JSON processing test |
| `/static/*` | GET | Static files | Serve CSS, JS, images |

## 🧪 Testing and Benchmarks

### Running Tests

```bash
# AES unit test
./build/Release/unit_test_aes.exe
# Output: SUCCESS: Assembly AES implementation is correct.

# SHA-256 unit test
./build/Release/unit_test_sha256.exe  
# Output: SUCCESS: Assembly SHA-256 implementation is correct.

# AES benchmark
./build/Release/benchmark_aes.exe
# Output: Starting Assembly AES-NI benchmark...
#         Processing 20000000 blocks (305 MB).
#         Finished in 0.08 seconds.
#         Throughput: 3.51 GB/s.

# SHA-256 benchmark
./build/Release/benchmark_sha256.exe
# Output: Starting Assembly SHA-256 benchmark...
#         Processing 5000000 blocks (305 MB).
```

### Test Coverage

- ✅ Assembly implementations (AES-NI, SHA-256)
- ✅ HTTP processing and routing
- ✅ JSON APIs and validation
- ✅ Buffer management and memory handling
- ✅ Static file serving
- ✅ Thread pool and concurrency
- ✅ Error handling and edge cases

## 📈 Performance and Benchmarks

### Typical Benchmarks

- **AES-NI Assembly**: 3.51 GB/s (20M blocks, 305 MB in 0.08s)
- **SHA-256 Assembly**: High performance with AVX registers
- **HTTP Processing**: < 1ms per request
- **Memory Usage**: Optimized with reusable buffer
- **Concurrent Connections**: Multiple simultaneous connections supported
- **Static File Serving**: High-throughput file delivery

### Optimizations Implemented

- Hand-coded assembly for AES-NI and AVX instructions
- Zero-copy buffer management with intelligent compaction
- Thread pool with automatic hardware detection
- Integrated OpenSSL provider for maximum compatibility
- Structured logging for debugging and monitoring
- Wildcard routing with efficient pattern matching

## 🔒 Security Features

- **TLS 1.3**: Latest encryption protocol
- **Custom Provider**: Auditable assembly implementations  
- **Input Validation**: Strict HTTP input validation
- **Buffer Overflow Protection**: Safe memory management
- **Structured Logging**: Complete security tracing
- **Error Handling**: No sensitive information exposure

## 🔄 Development

### Build System

The project uses CMake with support for:
- MSVC on Windows
- GCC/Clang on Linux
- Assembly compilation with NASM
- OpenSSL integration
- Automatic dependency detection

### Code Structure

```cpp
// Example: Custom buffer usage
https_server::Buffer buffer;
buffer.append("HTTP data", 9);
auto view = buffer.readable_view();
buffer.consume(4); // Zero-copy operations

// Example: JSON API endpoint
router.add_route("POST", "/api/data", [](const auto& req) {
    json data = json::parse(req.body);
    data["processed"] = true;
    
    https_server::http::HttpResponse response;
    response.body = data.dump(2);
    response.headers["Content-Type"] = "application/json";
    return response;
});
```

## 🚀 Production Deployment

### Performance Considerations

- Use Release build for production (`-DCMAKE_BUILD_TYPE=Release`)
- Configure appropriate thread count based on CPU cores
- Monitor memory usage with structured logs
- Set up proper TLS certificates
- Configure reverse proxy if needed (nginx, Apache)

### Monitoring

The server provides structured JSON logs:

```json
{
  "timestamp": "2025-01-15T10:30:00Z",
  "level": "INFO", 
  "message": "Request completed",
  "method": "POST",
  "uri": "/api/echo",
  "status_code": 200,
  "process_time_ms": 1.23
}
```

## 📚 Documentation

- **Modern Interface**: Responsive HTML design
- **API Documentation**: Well-documented endpoints
- **Code Comments**: Self-documenting code
- **Architecture Guide**: Clear modular structure
- **Performance Guides**: Optimization documentation

## 📜 License

Distributed under the MIT License. See `LICENSE` for more information.

## 📞 Contact

**Thiago Di Faria**
- Email: thiagodifaria@gmail.com
- GitHub: [@thiagodifaria](https://github.com/thiagodifaria)
- Project: [https://github.com/thiagodifaria/HTTPS-Server](https://github.com/thiagodifaria/HTTPS-Server)

---

⭐ **HTTPS Server** - Extreme performance with assembly-optimized cryptography and modern architecture.