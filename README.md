# HTTPS Server

![HTTPS Server Logo](https://img.shields.io/badge/HTTPS%20Server-High%20Performance-blue?style=for-the-badge&logo=cplusplus)

**High-Performance HTTPS Server with Assembly-Optimized Cryptography**

[![C++](https://img.shields.io/badge/C++-17-00599c?style=flat&logo=cplusplus&logoColor=white)](https://isocpp.org)
[![OpenSSL](https://img.shields.io/badge/OpenSSL-3.0-721412?style=flat&logo=openssl&logoColor=white)](https://openssl.org)
[![CMake](https://img.shields.io/badge/CMake-3.16+-064f8c?style=flat&logo=cmake&logoColor=white)](https://cmake.org)
[![NASM](https://img.shields.io/badge/NASM-Assembly-ff6600?style=flat)](https://nasm.us)
[![License](https://img.shields.io/badge/License-MIT-green.svg?style=flat)](LICENSE)
[![Windows](https://img.shields.io/badge/Windows-Ready-0078d4?style=flat&logo=windows&logoColor=white)](https://microsoft.com/windows)

---

## 🌍 **Documentation / Documentação**

**📖 [🇺🇸 Read in English](README_EN.md)**  
**📖 [🇧🇷 Leia em Português](README_PT.md)**

---

## 🎯 What is HTTPS Server?

A **production-ready HTTPS server** built in C++17 featuring custom **assembly-optimized cryptographic implementations** and a high-performance **OpenSSL provider**. Designed for maximum throughput, security, and efficiency in modern web applications.

### ⚡ Key Highlights

- 🔒 **Custom Crypto Provider** - Assembly-optimized AES-NI and SHA-256 implementations
- 🚀 **Extreme Performance** - 3.51 GB/s AES throughput, multi-threaded architecture
- 📊 **Modern APIs** - RESTful JSON endpoints with robust HTTP router
- 🛡️ **Production Ready** - TLS 1.3, secure buffer management, structured logging
- 🌐 **Cross Platform** - Windows and Linux support with CMake build system
- ⚙️ **Optimized Core** - Custom buffer class, thread pool, static file serving

### 🏆 What Makes It Special?

```
✅ Assembly-optimized cryptography for maximum performance
✅ Custom OpenSSL provider integration 
✅ Zero-copy buffer management with intelligent compaction
✅ Complete HTTP/1.1 implementation with routing
✅ Multi-threaded design with hardware concurrency detection
✅ JSON API support with nlohmann/json integration
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
# Test JSON API
curl -k -X POST "https://localhost:8443/api/echo" \
     -H "Content-Type: application/json" \
     -d '{"message": "Hello World!", "test": true}'

# Response:
# {
#   "message": "Hello World!",
#   "test": true,
#   "received": true,
#   "timestamp": 1234567890,
#   "server": "HTTPS Server v1.0"
# }
```

---

## 🔍 Performance Benchmarks

| Component | Throughput | Details |
|-----------|------------|---------|
| 🔐 **AES Encryption** | **3.51 GB/s** | 20M blocks, Assembly-optimized AES-NI |
| 🔑 **SHA-256 Hashing** | **High Performance** | Custom assembly implementation |
| 🌐 **HTTP Processing** | **Sub-millisecond** | Zero-copy buffer management |
| 🧵 **Threading** | **Hardware Optimized** | Automatic core detection |

*Benchmarks performed on Intel x64 architecture*

---

## 📞 Contact

**Thiago Di Faria** - thiagodifaria@gmail.com

[![GitHub](https://img.shields.io/badge/GitHub-@thiagodifaria-black?style=flat&logo=github)](https://github.com/thiagodifaria)

---

### 🌟 **Star this project if you find it useful!**

**Made with ❤️ by [Thiago Di Faria](https://github.com/thiagodifaria)**