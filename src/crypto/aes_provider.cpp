#include "aes.hpp"
#include <openssl/core_dispatch.h>
#include <openssl/core_names.h>
#include <openssl/params.h>
#include <openssl/evp.h>
#include <vector>
#include <openssl/aes.h>

struct prov_aes_ctx {
    std::vector<std::uint8_t> round_keys;
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

static void *aes_newctx(void *provctx) { 
    return new prov_aes_ctx(); 
}

static void aes_freectx(void *vctx) { 
    delete static_cast<prov_aes_ctx*>(vctx); 
}

static int aes_einit(void *vctx, const unsigned char *key, size_t keylen,
                     const unsigned char *iv, size_t ivlen, const OSSL_PARAM params[]) {
    auto *ctx = static_cast<prov_aes_ctx*>(vctx);
    if (key && keylen == 16) {
        ctx->round_keys.resize(176);
        AES_set_encrypt_key(key, 128, reinterpret_cast<AES_KEY*>(ctx->round_keys.data()));
    }
    return 1;
}

static const OSSL_DISPATCH aes128_ecb_functions[] = {
    { OSSL_FUNC_CIPHER_NEWCTX, reinterpret_cast<void (*)(void)>(aes_newctx) },
    { OSSL_FUNC_CIPHER_FREECTX, reinterpret_cast<void (*)(void)>(aes_freectx) },
    { OSSL_FUNC_CIPHER_ENCRYPT_INIT, reinterpret_cast<void (*)(void)>(aes_einit) },
    { OSSL_FUNC_CIPHER_CIPHER, reinterpret_cast<void (*)(void)>(aes128_cipher) },
    { 0, nullptr }
};

static const OSSL_ALGORITHM aes_ciphers[] = {
    { "AES-128-ECB", "provider=aes-ni", aes128_ecb_functions },
    { nullptr, nullptr, nullptr }
};

static const OSSL_ALGORITHM *aes_provider_query(void *provctx, int operation_id, int *no_cache) {
    *no_cache = 0;
    return (operation_id == OSSL_OP_CIPHER) ? aes_ciphers : nullptr;
}

static const OSSL_DISPATCH aes_provider_functions[] = {
    { OSSL_FUNC_PROVIDER_QUERY_OPERATION, reinterpret_cast<void (*)(void)>(aes_provider_query) },
    { 0, nullptr }
};

extern "C" OSSL_provider_init_fn OSSL_provider_init;
int OSSL_provider_init(const OSSL_CORE_HANDLE *handle, const OSSL_DISPATCH *in,
                       const OSSL_DISPATCH **out, void **provctx) {
    *out = aes_provider_functions;
    return 1;
}