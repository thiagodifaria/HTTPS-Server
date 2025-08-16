#ifdef _WIN32
#define _CRT_SECURE_NO_WARNINGS
#define NOMINMAX
#endif

#include "aes.hpp"
#include "sha256.hpp"
#include <openssl/core_dispatch.h>
#include <openssl/core_names.h>
#include <openssl/params.h>
#include <openssl/evp.h>
#include <vector>
#include <openssl/aes.h>
#include <cstring>
#include <algorithm>

struct prov_aes_ctx {
    std::vector<std::uint8_t> round_keys;
};

struct prov_sha256_ctx {
    std::vector<std::uint32_t> hash;
    std::vector<std::uint8_t> buffer;
    std::uint64_t total_len;
    size_t buffer_len;
};

static int aes128_cipher(void *vctx, unsigned char *out, size_t *outl,
                         size_t outsize, const unsigned char *in, size_t inl) {
    const auto *ctx = static_cast<prov_aes_ctx*>(vctx);
    
    if (inl != 16 || outsize < 16) {
        return 0;
    }

    aes_encrypt_block_asm(in, out, ctx->round_keys.data());
    *outl = 16;
    return 1;
}

static void *aes_newctx(void*) { 
    return new prov_aes_ctx(); 
}

static void aes_freectx(void *vctx) { 
    delete static_cast<prov_aes_ctx*>(vctx); 
}

static int aes_einit(void *vctx, const unsigned char *key, size_t keylen,
                     const unsigned char*, size_t, const OSSL_PARAM*) {
    auto *ctx = static_cast<prov_aes_ctx*>(vctx);
    if (key && keylen == 16) {
        ctx->round_keys.resize(176);
        AES_set_encrypt_key(key, 128, reinterpret_cast<AES_KEY*>(ctx->round_keys.data()));
    }
    return 1;
}

static void *sha256_newctx(void*) {
    auto *ctx = new prov_sha256_ctx();
    ctx->hash.resize(8);
    ctx->buffer.resize(64);
    ctx->hash[0] = 0x6a09e667;
    ctx->hash[1] = 0xbb67ae85;
    ctx->hash[2] = 0x3c6ef372;
    ctx->hash[3] = 0xa54ff53a;
    ctx->hash[4] = 0x510e527f;
    ctx->hash[5] = 0x9b05688c;
    ctx->hash[6] = 0x1f83d9ab;
    ctx->hash[7] = 0x5be0cd19;
    ctx->total_len = 0;
    ctx->buffer_len = 0;
    return ctx;
}

static void sha256_freectx(void *vctx) {
    delete static_cast<prov_sha256_ctx*>(vctx);
}

static int sha256_digest_init(void *vctx, const OSSL_PARAM*) {
    auto *ctx = static_cast<prov_sha256_ctx*>(vctx);
    // Reset to initial state
    ctx->hash[0] = 0x6a09e667;
    ctx->hash[1] = 0xbb67ae85;
    ctx->hash[2] = 0x3c6ef372;
    ctx->hash[3] = 0xa54ff53a;
    ctx->hash[4] = 0x510e527f;
    ctx->hash[5] = 0x9b05688c;
    ctx->hash[6] = 0x1f83d9ab;
    ctx->hash[7] = 0x5be0cd19;
    ctx->total_len = 0;
    ctx->buffer_len = 0;
    return 1;
}

static int sha256_digest_update(void *vctx, const unsigned char *in, size_t inl) {
    auto *ctx = static_cast<prov_sha256_ctx*>(vctx);
    
    ctx->total_len += inl;
    
    while (inl > 0) {
        const size_t copy_len = (std::min)(inl, 64 - ctx->buffer_len);
        std::memcpy(ctx->buffer.data() + ctx->buffer_len, in, copy_len);
        ctx->buffer_len += copy_len;
        in += copy_len;
        inl -= copy_len;
        
        if (ctx->buffer_len == 64) {
            sha256_block_asm(ctx->buffer.data(), ctx->hash.data());
            ctx->buffer_len = 0;
        }
    }
    
    return 1;
}

static int sha256_digest_final(void *vctx, unsigned char *out, size_t *outl, size_t outsize) {
    auto *ctx = static_cast<prov_sha256_ctx*>(vctx);
    
    if (outsize < 32) {
        return 0;
    }
    
    ctx->buffer[ctx->buffer_len++] = 0x80;
    
    if (ctx->buffer_len > 56) {
        while (ctx->buffer_len < 64) {
            ctx->buffer[ctx->buffer_len++] = 0x00;
        }
        sha256_block_asm(ctx->buffer.data(), ctx->hash.data());
        ctx->buffer_len = 0;
    }
    
    while (ctx->buffer_len < 56) {
        ctx->buffer[ctx->buffer_len++] = 0x00;
    }
    
    const std::uint64_t bit_len = ctx->total_len * 8;
    for (int i = 7; i >= 0; --i) {
        ctx->buffer[56 + i] = static_cast<std::uint8_t>(bit_len >> (8 * (7 - i)));
    }
    
    sha256_block_asm(ctx->buffer.data(), ctx->hash.data());
    
    for (int i = 0; i < 8; ++i) {
        const std::uint32_t h = ctx->hash[i];
        out[i * 4 + 0] = static_cast<std::uint8_t>(h >> 24);
        out[i * 4 + 1] = static_cast<std::uint8_t>(h >> 16);
        out[i * 4 + 2] = static_cast<std::uint8_t>(h >> 8);
        out[i * 4 + 3] = static_cast<std::uint8_t>(h);
    }
    
    *outl = 32;
    return 1;
}

static const OSSL_DISPATCH aes128_ecb_functions[] = {
    { OSSL_FUNC_CIPHER_NEWCTX, reinterpret_cast<void (*)(void)>(aes_newctx) },
    { OSSL_FUNC_CIPHER_FREECTX, reinterpret_cast<void (*)(void)>(aes_freectx) },
    { OSSL_FUNC_CIPHER_ENCRYPT_INIT, reinterpret_cast<void (*)(void)>(aes_einit) },
    { OSSL_FUNC_CIPHER_CIPHER, reinterpret_cast<void (*)(void)>(aes128_cipher) },
    { 0, nullptr }
};

static const OSSL_DISPATCH sha256_functions[] = {
    { OSSL_FUNC_DIGEST_NEWCTX, reinterpret_cast<void (*)(void)>(sha256_newctx) },
    { OSSL_FUNC_DIGEST_FREECTX, reinterpret_cast<void (*)(void)>(sha256_freectx) },
    { OSSL_FUNC_DIGEST_INIT, reinterpret_cast<void (*)(void)>(sha256_digest_init) },
    { OSSL_FUNC_DIGEST_UPDATE, reinterpret_cast<void (*)(void)>(sha256_digest_update) },
    { OSSL_FUNC_DIGEST_FINAL, reinterpret_cast<void (*)(void)>(sha256_digest_final) },
    { 0, nullptr }
};

static const OSSL_ALGORITHM algorithms[] = {
    { "AES-128-ECB", "provider=aes-ni", aes128_ecb_functions },
    { "SHA256", "provider=sha256-avx", sha256_functions },
    { nullptr, nullptr, nullptr }
};

static const OSSL_ALGORITHM *provider_query(void*, int operation_id, int *no_cache) {
    *no_cache = 0;
    
    switch (operation_id) {
        case OSSL_OP_CIPHER:
            return &algorithms[0]; // AES algorithms
        case OSSL_OP_DIGEST:
            return &algorithms[1]; // SHA-256 algorithms
        default:
            return nullptr;
    }
}

static const OSSL_DISPATCH provider_functions[] = {
    { OSSL_FUNC_PROVIDER_QUERY_OPERATION, reinterpret_cast<void (*)(void)>(provider_query) },
    { 0, nullptr }
};

extern "C" OSSL_provider_init_fn OSSL_provider_init;
int OSSL_provider_init(const OSSL_CORE_HANDLE*, const OSSL_DISPATCH*,
                       const OSSL_DISPATCH **out, void**) {
    *out = provider_functions;
    return 1;
}