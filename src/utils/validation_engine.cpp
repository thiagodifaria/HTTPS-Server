#include "utils/validation_engine.hpp"
#include <algorithm>

#ifdef _WIN32
#include <intrin.h>
#else
#include <cpuid.h>
#endif

namespace https_server {
namespace validation {

bool ValidationOps::detect_avx2() noexcept {
#ifdef _WIN32
    int cpuInfo[4];
    __cpuid(cpuInfo, 7);
    return (cpuInfo[1] & (1 << 5)) != 0;
#else
    uint32_t eax, ebx, ecx, edx;
    if (__get_cpuid_max(0, nullptr) >= 7) {
        __cpuid_count(7, 0, eax, ebx, ecx, edx);
        return (ebx & (1u << 5)) != 0;
    }
    return false;
#endif
}

ValidationResult ValidationOps::fallback_json_validate(const char* data, size_t len) const noexcept {
    if (!data || len == 0) return ValidationResult::INVALID_JSON;
    
    int brace_count = 0;
    int bracket_count = 0;
    bool in_string = false;
    
    for (size_t i = 0; i < len; ++i) {
        unsigned char c = static_cast<unsigned char>(data[i]);
        
        if (c > 127) {
            if (!fallback_utf8_validate(data + i, len - i)) {
                return ValidationResult::INVALID_UTF8;
            }
        }
        
        if (!in_string) {
            if (c == '{') brace_count++;
            else if (c == '}') brace_count--;
            else if (c == '[') bracket_count++;
            else if (c == ']') bracket_count--;
            else if (c == '"') in_string = true;
            
            if (brace_count < 0 || bracket_count < 0) {
                return ValidationResult::INVALID_JSON;
            }
        } else {
            if (c == '"' && (i == 0 || data[i-1] != '\\')) {
                in_string = false;
            }
        }
    }
    
    return (brace_count == 0 && bracket_count == 0 && !in_string) ? 
           ValidationResult::VALID : ValidationResult::INVALID_JSON;
}

bool ValidationOps::fallback_sanitize(char* data, size_t len) const noexcept {
    if (!data) return false;
    
    for (size_t i = 0; i < len; ++i) {
        char c = data[i];
        if (c == '<' || c == '>' || c == '&' || c == '"' || c == '\'' || 
            c == '\0' || c == '\n' || c == '\r') {
            data[i] = '_';
        }
    }
    
    return true;
}

bool ValidationOps::fallback_utf8_validate(const char* data, size_t len) const noexcept {
    if (!data) return false;
    
    for (size_t i = 0; i < len; ++i) {
        unsigned char c = static_cast<unsigned char>(data[i]);
        
        if (c < 0x80) {
            continue;
        } else if (c < 0xC0) {
            return false;
        } else if (c < 0xE0) {
            if (i + 1 >= len) return false;
            if ((static_cast<unsigned char>(data[i + 1]) & 0xC0) != 0x80) return false;
            i++;
        } else if (c < 0xF0) {
            if (i + 2 >= len) return false;
            if ((static_cast<unsigned char>(data[i + 1]) & 0xC0) != 0x80) return false;
            if ((static_cast<unsigned char>(data[i + 2]) & 0xC0) != 0x80) return false;
            i += 2;
        } else if (c < 0xF8) {
            if (i + 3 >= len) return false;
            if ((static_cast<unsigned char>(data[i + 1]) & 0xC0) != 0x80) return false;
            if ((static_cast<unsigned char>(data[i + 2]) & 0xC0) != 0x80) return false;
            if ((static_cast<unsigned char>(data[i + 3]) & 0xC0) != 0x80) return false;
            i += 3;
        } else {
            return false;
        }
    }
    
    return true;
}

} // namespace validation
} // namespace https_server