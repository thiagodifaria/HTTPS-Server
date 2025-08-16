# HTTPS Server - Servidor HTTPS de Alta Performance

Servidor HTTPS avançado implementado em C++17 com implementações criptográficas otimizadas em assembly e provider OpenSSL de alta performance. Este projeto oferece uma solução completa para aplicações web de alto throughput, incluindo criptografia assembly hand-coded, gerenciamento de buffer de baixo nível, APIs RESTful JSON, e arquitetura multi-threaded robusta.

## 🎯 Funcionalidades

- ✅ **Provider criptográfico customizado**: Implementações assembly otimizadas para AES-NI e SHA-256
- ✅ **Performance extrema**: Throughput AES de 3.51 GB/s com arquitetura multi-threaded
- ✅ **APIs modernas**: Endpoints JSON RESTful com roteador HTTP robusto
- ✅ **Buffer de alta performance**: Gerenciamento de memória zero-copy com compactação inteligente
- ✅ **Produção ready**: TLS 1.3, logging estruturado, thread pool otimizado
- ✅ **Cross-platform**: Suporte Windows e Linux com sistema de build CMake
- ✅ **Arquitetura modular**: Separação clara de responsabilidades e componentes testáveis

## 🗂️ Arquitetura

Arquitetura modular com separação clara de responsabilidades:

```
src/
├── core/           # Infraestrutura (servidor, config, thread pool)
├── crypto/         # Engine criptográfico e assembly otimizado  
├── http/           # Protocolo HTTP, roteamento e arquivos estáticos
├── utils/          # Buffer de performance, logging e utilitários
└── main.cpp        # Ponto de entrada e configuração de rotas
```

## 🔧 Stack Tecnológico

### Tecnologias Core
- **C++17**: C++ moderno com recursos avançados
- **OpenSSL 3.0**: Biblioteca criptográfica e implementação TLS
- **CMake**: Sistema de build cross-platform
- **NASM**: Assembler x86-64 para otimizações de performance

### Criptografia
- **AES-NI Assembly**: Implementação hand-optimized usando instruções Intel AES
- **SHA-256 Assembly**: Hash criptográfico com registradores AVX
- **Custom OpenSSL Provider**: Integração nativa com stack OpenSSL

### Performance
- **Thread Pool**: Pool multi-threaded com detecção automática de cores
- **Zero-copy Buffer**: Classe de buffer customizada com reutilização de memória
- **Structured Logging**: Sistema de logs JSON estruturados

### APIs e HTTP
- **nlohmann/json**: Processamento JSON de alta performance
- **HTTP Router**: Sistema de roteamento com suporte a wildcards
- **Static File Server**: Servidor de arquivos estáticos otimizado

## 📋 Pré-requisitos

- Compilador compatível com C++17+ (MSVC 2022, GCC 9+, Clang 10+)
- CMake 3.16+
- OpenSSL 3.0+
- NASM (para compilação assembly)

## 🚀 Instalação Rápida

### Windows (Recomendado)

```bash
# Clonar repositório
git clone https://github.com/thiagodifaria/HTTPS-Server.git
cd HTTPS-Server

# Build com PowerShell
./build.ps1

# Executar servidor
./build/Release/https_server.exe
```

### Linux/Unix

```bash
# Clonar repositório
git clone https://github.com/thiagodifaria/HTTPS-Server.git
cd HTTPS-Server

# Configurar e compilar
mkdir build && cd build
cmake .. -DCMAKE_BUILD_TYPE=Release
make -j$(nproc)

# Executar servidor
./https_server
```

## ⚙️ Configuração

### Configuração do Ambiente

O servidor usa um arquivo `config.json` para configuração:

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

### Certificados TLS

```bash
# Gerar certificado auto-assinado para desenvolvimento
openssl req -x509 -newkey rsa:4096 -keyout key.pem -out cert.pem -days 365 -nodes
```

## 📊 Uso da API

### Página Principal

```bash
curl -k https://localhost:8443/
# Retorna interface HTML moderna com design interativo
```

### API JSON Echo

```bash
curl -k -X POST "https://localhost:8443/api/echo" \
     -H "Content-Type: application/json" \
     -d '{"mensagem": "Olá Mundo!", "timestamp": 1640995200}'
```

**Resposta:**
```json
{
  "mensagem": "Olá Mundo!",
  "timestamp": 1640995200,
  "received": true,
  "timestamp": 1234567890,
  "server": "HTTPS Server v1.0"
}
```

### Servir Arquivos Estáticos

```bash
# Servir arquivos da pasta public/
curl -k https://localhost:8443/static/test.html
curl -k https://localhost:8443/static/style.css
```

### Tratamento de Erros

```bash
# Tratamento de 404
curl -k https://localhost:8443/inexistente
# Retorna página 404 estilizada

# Validação JSON
curl -k -X POST "https://localhost:8443/api/echo" \
     -H "Content-Type: application/json" \
     -d 'json inválido'
# Retorna resposta de erro em JSON
```

## 🛠️ Endpoints Principais

| Endpoint | Método | Descrição | Funcionalidade |
|----------|--------|-----------|----------------|
| `/` | GET | Página principal | Interface HTML moderna |
| `/about` | GET | Informações do servidor | Detalhes técnicos |
| `/api/echo` | POST | API JSON echo | Teste de processamento JSON |
| `/static/*` | GET | Arquivos estáticos | Servir CSS, JS, imagens |

## 🧪 Testes e Benchmarks

### Executar Testes

```bash
# Teste unitário AES
./build/Release/unit_test_aes.exe
# Output: SUCCESS: Assembly AES implementation is correct.

# Teste unitário SHA-256  
./build/Release/unit_test_sha256.exe
# Output: SUCCESS: Assembly SHA-256 implementation is correct.

# Benchmark AES
./build/Release/benchmark_aes.exe
# Output: Starting Assembly AES-NI benchmark...
#         Processing 20000000 blocks (305 MB).
#         Finished in 0.08 seconds.
#         Throughput: 3.51 GB/s.

# Benchmark SHA-256
./build/Release/benchmark_sha256.exe
# Output: Starting Assembly SHA-256 benchmark...
#         Processing 5000000 blocks (305 MB).
```

### Cobertura de Testes

- ✅ Implementações assembly (AES-NI, SHA-256)
- ✅ Processamento HTTP e roteamento  
- ✅ APIs JSON e validação
- ✅ Gerenciamento de buffer e memória
- ✅ Servir arquivos estáticos
- ✅ Thread pool e concorrência
- ✅ Tratamento de erros e casos extremos

## 📈 Performance e Benchmarks

### Benchmarks Típicos

- **AES-NI Assembly**: 3.51 GB/s (20M blocos, 305 MB em 0.08s)
- **SHA-256 Assembly**: Alta performance com registradores AVX
- **Processamento HTTP**: < 1ms por requisição
- **Uso de Memória**: Otimizado com buffer reutilizável
- **Conexões Concorrentes**: Múltiplas conexões simultâneas suportadas
- **Servir Arquivos Estáticos**: Entrega de arquivos com alto throughput

### Otimizações Implementadas

- Assembly hand-coded para instruções AES-NI e AVX
- Gerenciamento de buffer zero-copy com compactação inteligente
- Thread pool com detecção automática de hardware
- Provider OpenSSL integrado para máxima compatibilidade
- Logs estruturados para debugging e monitoramento
- Roteamento com wildcards e pattern matching eficiente

## 🔒 Recursos de Segurança

- **TLS 1.3**: Protocolo de criptografia mais recente
- **Custom Provider**: Implementações assembly auditáveis
- **Validação de Entrada**: Validação rigorosa de entrada HTTP
- **Proteção Buffer Overflow**: Gerenciamento seguro de memória
- **Logging Estruturado**: Rastreamento completo de segurança
- **Tratamento de Erros**: Não exposição de informações sensíveis

## 🔄 Desenvolvimento

### Sistema de Build

O projeto usa CMake com suporte para:
- MSVC no Windows
- GCC/Clang no Linux
- Compilação assembly com NASM
- Integração OpenSSL
- Detecção automática de dependências

### Estrutura do Código

```cpp
// Exemplo: Uso do buffer customizado
https_server::Buffer buffer;
buffer.append("Dados HTTP", 10);
auto view = buffer.readable_view();
buffer.consume(4); // Operações zero-copy

// Exemplo: Endpoint de API JSON
router.add_route("POST", "/api/dados", [](const auto& req) {
    json dados = json::parse(req.body);
    dados["processado"] = true;
    
    https_server::http::HttpResponse response;
    response.body = dados.dump(2);
    response.headers["Content-Type"] = "application/json";
    return response;
});
```

## 🚀 Deploy em Produção

### Considerações de Performance

- Use build Release para produção (`-DCMAKE_BUILD_TYPE=Release`)
- Configure número apropriado de threads baseado nos cores da CPU
- Monitore uso de memória com logs estruturados
- Configure certificados TLS apropriados
- Configure reverse proxy se necessário (nginx, Apache)

### Monitoramento

O servidor fornece logs JSON estruturados:

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

## 📚 Documentação

- **Interface Moderna**: Design HTML responsivo
- **Documentação da API**: Endpoints bem documentados
- **Comentários no Código**: Código auto-documentado
- **Guia de Arquitetura**: Estrutura modular clara
- **Guias de Performance**: Documentação de otimização

## 📜 Licença

Distribuído sob a licença MIT. Veja `LICENSE` para mais informações.

## 📞 Contato

**Thiago Di Faria**
- Email: thiagodifaria@gmail.com
- GitHub: [@thiagodifaria](https://github.com/thiagodifaria)
- Projeto: [https://github.com/thiagodifaria/HTTPS-Server](https://github.com/thiagodifaria/HTTPS-Server)

---

⭐ **HTTPS Server** - Performance extrema com criptografia assembly otimizada e arquitetura moderna.