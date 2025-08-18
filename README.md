# HTTPS Server

![HTTPS Server Logo](https://img.shields.io/badge/HTTPS%20Server-High%20Performance-blue?style=for-the-badge&logo=cplusplus)

**High-Performance HTTPS Server with SIMD-Optimized Network Operations**

[![C++](https://img.shields.io/badge/C++-17-00599c?style=flat&logo=cplusplus&logoColor=white)](https://isocpp.org)
[![OpenSSL](https://img.shields.io/badge/OpenSSL-3.0-721412?style=flat&logo=openssl&logoColor=white)](https://openssl.org)
[![CMake](https://img.shields.io/badge/CMake-3.16+-064f8c?style=flat&logo=cmake&logoColor=white)](https://cmake.org)
[![NASM](https://img.shields.io/badge/NASM-Assembly-ff6600?style=flat)](https://nasm.us)
[![License](https://img.shields.io/badge/License-MIT-green.svg?style=flat)](LICENSE)
[![Windows](https://img.shields.io/badge/Windows-Ready-0078d4?style=flat&logo=windows&logoColor=white)](https://microsoft.com/windows)

---

## 🌐 **Documentation / Documentação**

**📖 [🇺🇸 Read in English](README_EN.md)**  
**📖 [🇧🇷 Leia em Português](README_PT.md)**

---

## 🎯 What is HTTPS Server?

A **production-ready HTTPS server** built in C++17 featuring comprehensive **SIMD-optimized network operations** and **advanced assembly cryptographic implementations**. Designed for maximum throughput, security, and efficiency in modern web applications.

### ⚡ Key Highlights

- 🔒 **Advanced Cryptography** - AES-NI, SHA-256 AVX2, ChaCha20, Blake3, X25519
- 🚀 **SIMD Network Operations** - Base64 vectorized, UUID v4 with hardware RNG, hex encoding
- 📊 **HTTP Performance** - AVX2 parsing acceleration, validation engine, compression suite  
- 🛡️ **Production Ready** - TLS 1.3, input validation SIMD, structured logging
- 🌐 **Cross Platform** - Windows and Linux support with automatic CPU detection
- ⚙️ **Optimized Core** - Deflate/LZ4/Brotli compression, zero-copy buffer management

### 🏆 What Makes It Special?

```
✅ HTTP parsing accelerated with AVX2 (\r\n\r\n detection)
✅ JSON validation engine with SIMD character classification
✅ Advanced crypto: ChaCha20-Poly1305, Blake3, X25519
✅ Compression suite: Deflate, LZ4, Brotli optimized
✅ Network operations: Base64 SIMD, UUID v4 RDRAND, hex vectorized
✅ Complete performance benchmark suite accessible via web interface
```

---

## ⚡ Quick Start

### Prerequisites
- **C++17** compatible compiler (MSVC 2022, GCC 9+, Clang 10+)
- **CMake 3.16+**
- **OpenSSL 3.0+**
- **NASM** for assembly compilation

### Build and Run
```bash
# Clone repository
git clone https://github.com/thiagodifaria/HTTPS-Server.git
cd HTTPS-Server

# Windows (PowerShell)
./build.ps1

# Linux/Unix
mkdir build && cd build
cmake .. -DCMAKE_BUILD_TYPE=Release
make -j$(nproc)

# Run server
./https_server
# Server starts at https://localhost:8443
```

### 🔥 Test It Now!
```bash
# Test JSON API with SIMD validation
curl -k -X POST "https://localhost:8443/api/echo" \
     -H "Content-Type: application/json" \
     -d '{"message": "Hello World!", "encode_data": "test"}'

# Response includes Base64/Hex encoding:
# {
#   "message": "Hello World!",
#   "encode_data": "test",
#   "base64_encoded": "dGVzdA==",
#   "hex_encoded": "74657374",
#   "received": true,
#   "server": "HTTPS Server v1.0"
# }

# Performance benchmarks
curl -k https://localhost:8443/api/benchmark
```

---

## 🔥 Performance Benchmarks

| Component | Throughput | Details |
|-----------|------------|---------|
| 🔐 **AES-NI Assembly** | **3.51 GB/s** | 20M blocks, hand-optimized implementation |
| 🔒 **SHA-256 AVX** | **High Performance** | Vectorized hash computation |
| 🌐 **HTTP Parsing** | **Sub-millisecond** | AVX2 accelerated \r\n\r\n detection |
| 📊 **Base64 SIMD** | **Vectorized** | VPSHUFB lookup table encoding |
| 🎲 **UUID Generation** | **Hardware RNG** | RDRAND instruction when available |
| 🗜️ **Compression** | **Multi-algorithm** | Deflate, LZ4, Brotli optimized |

*Access real-time benchmarks at https://localhost:8443/bench*

---

## 🛠️ SIMD Modules

### 📡 Network Operations
- **Base64 SIMD**: Vectorized encoding/decoding with lookup tables
- **UUID v4**: Hardware RNG (RDRAND) with assembly fallback
- **Hex Encoding**: Optimized binary to hex string conversion

### 🔍 Validation Engine  
- **JSON SIMD**: Fast validation without complete parsing
- **UTF-8 Vectorized**: Character encoding validation
- **Input Sanitization**: SIMD character class detection

### 🗜️ Compression Suite
- **Deflate**: Optimized for files < 64KB
- **LZ4**: Ultra-fast compression
- **Brotli**: Web content optimization (HTML/CSS/JS)

### ⚡ HTTP Acceleration
- **AVX2 Parsing**: 32-byte simultaneous processing
- **Header Detection**: Vectorized \r\n\r\n search
- **Method/URI Extraction**: Accelerated request parsing

---

## 📞 Contact

**Thiago Di Faria** - thiagodifaria@gmail.com

[![GitHub](https://img.shields.io/badge/GitHub-@thiagodifaria-black?style=flat&logo=github)](https://github.com/thiagodifaria)

---

### 🌟 **Star this project if you find it useful!**

**Made with ❤️ by [Thiago Di Faria](https://github.com/thiagodifaria)**