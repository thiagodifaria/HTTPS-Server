# HTTPS Server - High-Performance C++ Web Server

Advanced HTTPS server implementation in C++17 featuring comprehensive SIMD-optimized network operations, HTTP parsing acceleration, validation engines, compression algorithms, and advanced cryptographic implementations. This project delivers a complete solution for high-throughput web applications with hand-coded assembly optimizations.

## 🎯 Features

- ✅ **SIMD Network Operations**: Base64 vectorized, UUID v4 hardware RNG, hex encoding optimized
- ✅ **HTTP Parsing Acceleration**: AVX2 \r\n\r\n detection, method/URI extraction, header counting
- ✅ **Validation Engine**: JSON SIMD validation, UTF-8 vectorized, input sanitization
- ✅ **Advanced Cryptography**: ChaCha20-Poly1305, Blake3, X25519, AES-NI, SHA-256 AVX
- ✅ **Compression Suite**: Deflate, LZ4, Brotli with sliding window optimization
- ✅ **Performance Benchmarks**: Real-time web interface for testing all optimizations
- ✅ **Production Ready**: TLS 1.3, structured logging, thread pool, cross-platform

## 🗂️ Architecture

Modular architecture with SIMD-optimized components:

```
src/
├── core/           # Infrastructure (server, config, thread pool)
├── crypto/         # Advanced crypto: ChaCha20, Blake3, X25519, AES-NI
├── http/           # HTTP acceleration, compression, static serving
├── utils/          # Network ops, validation, compression SIMD
└── main.cpp        # Entry point and benchmark API
```

## 🔧 Technology Stack

### SIMD Optimizations
- **AVX2**: 32-byte parallel processing for HTTP parsing
- **VPSHUFB**: Base64 character lookup tables
- **RDRAND**: Hardware random number generation for UUID v4
- **Character Classification**: SIMD validation engines

### Advanced Cryptography
- **ChaCha20-Poly1305**: Modern authenticated encryption
- **Blake3**: Tree hashing with SIMD optimizations
- **X25519**: Montgomery ladder scalar multiplication
- **AES-NI + SHA-256**: Hand-optimized assembly implementations

### Compression Algorithms
- **Deflate**: Optimized for small files with hash tables
- **LZ4**: Ultra-fast compression with string matching
- **Brotli**: Web content optimization for HTML/CSS/JS

### Core Technologies
- **C++17**: Modern C++ with advanced features
- **OpenSSL 3.0**: Extended with custom provider
- **CMake**: Cross-platform build system
- **NASM**: x86-64 assembler for optimizations

## 📋 Prerequisites

- C++17+ compatible compiler (MSVC 2022, GCC 9+, Clang 10+)
- CMake 3.16+
- OpenSSL 3.0+
- NASM (for assembly compilation)
- CPU with AVX2 support (recommended for full performance)

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

### CPU Feature Detection

The server automatically detects and enables:
- **AVX2**: For HTTP parsing and Base64 operations
- **RDRAND**: For hardware UUID generation
- **AES-NI**: For cryptographic acceleration

## 📊 API Usage

### Performance Benchmarks

```bash
# Real-time benchmark execution
curl -k https://localhost:8443/api/benchmark

# Returns performance data:
# {
#   "aes_ni": {"throughput": "3.51 GB/s", "time": "0.08s"},
#   "sha256": {"throughput": "2.1 GB/s", "time": "0.12s"},
#   "p256": {"field_ops": 850000, "ecdh_est": 1660}
# }
```

### JSON API with SIMD Validation

```bash
curl -k -X POST "https://localhost:8443/api/echo" \
     -H "Content-Type: application/json" \
     -d '{"message": "Hello World!", "encode_data": "test data"}'
```

**Response with network operations:**
```json
{
  "message": "Hello World!",
  "encode_data": "test data",
  "base64_encoded": "dGVzdCBkYXRh",
  "hex_encoded": "7465737420646174612020",
  "received": true,
  "timestamp": 1234567890,
  "server": "HTTPS Server v1.0"
}
```

### Web Performance Interface

```bash
# Access benchmark interface
curl -k https://localhost:8443/bench
# Interactive web interface for running performance tests

# Main interface with all features
curl -k https://localhost:8443/
# Modern HTML interface showcasing all optimizations
```

## 🛠️ Main Endpoints

| Endpoint | Method | Description | SIMD Features |
|----------|--------|-------------|---------------|
| `/` | GET | Main interface | HTTP parsing acceleration |
| `/about` | GET | Technical details | All optimization info |
| `/bench` | GET | Benchmark interface | Real-time performance testing |
| `/api/echo` | POST | JSON processing | SIMD validation + network ops |
| `/api/benchmark` | GET | Performance API | All algorithm benchmarks |

## 🧪 Testing and Benchmarks

### Web-Based Benchmarks

```bash
# Access interactive benchmarks
https://localhost:8443/bench

# API endpoint for automated testing
curl -k https://localhost:8443/api/benchmark
```

### Command Line Tests

```bash
# AES assembly benchmark
./build/Release/benchmark_aes.exe
# Output: AES-NI Assembly: 3.51 GB/s (20M blocks, 305 MB in 0.08s)

# SHA-256 assembly benchmark
./build/Release/benchmark_sha256.exe
# Output: SHA-256 Assembly: High performance with AVX registers

# P-256 elliptic curve benchmark
./build/Release/benchmark_p256.exe
# Output: Field operations, point arithmetic, ECDH estimates
```

### SIMD Feature Testing

The server logs SIMD capability detection:

```
[Info] AVX2 memory optimizations enabled
[Info] HTTP parsing optimizations enabled  
[Info] Network operations optimized (Base64/UUID/Hex with RDRAND+AVX2)
[Info] Advanced crypto algorithms available (ChaCha20, Blake3, X25519)
[Info] Compression optimizations enabled (Deflate/LZ4/Brotli)
```

## 📈 Performance and Benchmarks

### Real-Time Performance

Access live benchmarks at `https://localhost:8443/bench`:

- **AES-NI Assembly**: 3.51 GB/s throughput
- **SHA-256 AVX**: Vectorized hash computation
- **HTTP Parsing**: AVX2 accelerated \r\n\r\n detection
- **Base64 SIMD**: VPSHUFB lookup table operations
- **UUID Generation**: Hardware RDRAND when available
- **Compression**: Multi-algorithm optimization

### SIMD Optimizations Implemented

- **HTTP Parsing**: VPCMPEQB for 32-byte pattern matching
- **Base64 Operations**: VPSHUFB character lookup tables  
- **Validation Engine**: Character class detection with SIMD
- **Compression**: Sliding window with hash tables
- **Network Operations**: Batch processing for arrays

## 🔒 Security Features

- **TLS 1.3**: Latest encryption protocol
- **SIMD Validation**: Fast input sanitization and JSON validation
- **Advanced Crypto**: ChaCha20-Poly1305, Blake3, X25519 algorithms
- **Hardware RNG**: RDRAND for secure UUID generation
- **Input Validation**: SIMD character class detection
- **Buffer Protection**: Safe memory management with zero-copy operations

## 📄 Development

### SIMD Module Structure

```cpp
// Example: Network operations usage
auto& net_ops = network_ops::NetworkOps::instance();
if (net_ops.has_avx2() && net_ops.has_rdrand()) {
    // Hardware-accelerated operations
    std::string encoded = net_ops.encode_base64(data);
    uint8_t uuid[16];
    net_ops.uuid_generate_v4(uuid);
}

// Example: HTTP parsing acceleration
if (http_accelerated::HttpOps::instance().has_avx2()) {
    size_t header_end;
    bool found = http_ops.find_header_end(data, len, &header_end);
}

// Example: Validation with SIMD
auto result = validation::ValidationOps::instance()
    .json_validate_fast(json_data.c_str(), json_data.size());
```

### Benchmark Integration

```cpp
// Web-accessible performance testing
router.add_route("GET", "/api/benchmark", [](const auto& req) {
    auto aes_result = benchmark::run_aes_benchmark();
    auto sha_result = benchmark::run_sha256_benchmark();
    auto p256_result = benchmark::run_p256_benchmark();
    
    json response;
    response["aes_ni"]["throughput"] = format_throughput(aes_result);
    response["sha256"]["throughput"] = format_throughput(sha_result);
    response["p256"]["field_ops"] = p256_result.field_ops_per_sec;
    
    return json_response(response);
});
```

## 🚀 Production Deployment

### Performance Considerations

- Enable all SIMD optimizations with Release build
- Verify CPU capabilities (AVX2, RDRAND, AES-NI)
- Configure appropriate thread count for workload
- Monitor performance via `/api/benchmark` endpoint
- Use compression for static content delivery

### SIMD Capability Monitoring

The server provides comprehensive capability detection:

```json
{
  "cpu_features": {
    "avx2": true,
    "rdrand": true,
    "aes_ni": true
  },
  "optimizations": {
    "http_parsing": "enabled",
    "network_operations": "full",
    "validation_engine": "simd",
    "compression": "multi-algorithm"
  }
}
```

## 📚 Documentation

- **Web Interface**: Interactive demonstration of all SIMD features
- **Benchmark Suite**: Real-time performance testing via web browser
- **API Documentation**: Comprehensive endpoint documentation
- **Architecture Guide**: SIMD optimization documentation
- **Performance Analysis**: Detailed optimization explanations

## 📜 License

Distributed under the MIT License. See `LICENSE` for more information.

## 📞 Contact

**Thiago Di Faria**
- Email: thiagodifaria@gmail.com
- GitHub: [@thiagodifaria](https://github.com/thiagodifaria)
- Project: [https://github.com/thiagodifaria/HTTPS-Server](https://github.com/thiagodifaria/HTTPS-Server)

---

⭐ **HTTPS Server** - Extreme performance with comprehensive SIMD optimizations and modern architecture.