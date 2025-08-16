#include "utils/fast_memory.hpp"
#include <iostream>
#include <vector>
#include <cstring>
#include <random>

int main() {
    std::cout << "Fast Memory Operations Test" << std::endl;
    
    const auto& mem_ops = https_server::fast_memory::MemoryOps::instance();
    
    std::cout << "AVX2 Support: " << (mem_ops.has_avx2() ? "YES" : "NO") << std::endl;
    
    constexpr size_t test_size = 1024;
    std::vector<char> src(test_size);
    std::vector<char> dst(test_size);
    std::vector<char> reference(test_size);
    
    std::mt19937 rng(42);
    std::uniform_int_distribution<int> dist(0, 255);
    for (size_t i = 0; i < test_size; ++i) {
        src[i] = static_cast<char>(dist(rng));
    }
    
    std::cout << "\nTesting memcpy..." << std::endl;
    std::memcpy(reference.data(), src.data(), test_size);
    https_server::fast_memory::memcpy(dst.data(), src.data(), test_size);
    
    if (std::memcmp(dst.data(), reference.data(), test_size) == 0) {
        std::cout << "PASS: memcpy produces correct results" << std::endl;
    } else {
        std::cout << "FAIL: memcpy produces incorrect results" << std::endl;
        return 1;
    }
    
    std::cout << "Testing memchr..." << std::endl;
    const char search_val = src[test_size / 2];
    
    void* std_result = std::memchr(src.data(), search_val, test_size);
    void* fast_result = https_server::fast_memory::memchr(src.data(), search_val, test_size);
    
    if (std_result == fast_result) {
        std::cout << "PASS: memchr produces correct results" << std::endl;
    } else {
        std::cout << "FAIL: memchr produces incorrect results" << std::endl;
        return 1;
    }
    
    std::cout << "Testing memmove..." << std::endl;
    std::vector<char> overlap_test(test_size);
    std::vector<char> overlap_ref(test_size);
    
    std::memcpy(overlap_test.data(), src.data(), test_size);
    std::memcpy(overlap_ref.data(), src.data(), test_size);
    
    std::memmove(overlap_ref.data() + 10, overlap_ref.data(), test_size - 10);
    https_server::fast_memory::memmove(overlap_test.data() + 10, overlap_test.data(), test_size - 10);
    
    if (std::memcmp(overlap_test.data(), overlap_ref.data(), test_size) == 0) {
        std::cout << "PASS: memmove produces correct results" << std::endl;
    } else {
        std::cout << "FAIL: memmove produces incorrect results" << std::endl;
        return 1;
    }
    
    std::cout << "\nSUCCESS: All fast memory tests passed." << std::endl;
    return 0;
}