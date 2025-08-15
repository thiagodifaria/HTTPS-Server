cmake_minimum_required(VERSION 3.16)
project(https_server LANGUAGES CXX ASM_NASM)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

find_package(OpenSSL REQUIRED)

add_subdirectory(src/crypto)

add_executable(https_server 
    src/main.cpp
    src/core/server.cpp
    src/core/config.cpp
    src/utils/logger.cpp
    src/http/static_handler.cpp
    src/crypto/aes_provider.cpp
)

target_compile_definitions(https_server PRIVATE OPENSSL_ROOT_DIR="${OPENSSL_ROOT_DIR}")
target_include_directories(https_server PRIVATE src third_party ${OPENSSL_INCLUDE_DIR})
target_link_libraries(https_server PRIVATE aes_asm_impl OpenSSL::SSL OpenSSL::Crypto)
if(WIN32)
    target_link_libraries(https_server PRIVATE ws2_32)
endif()

add_library(aes_asm_impl src/crypto/asm/x86_64/aes_ni.asm)

add_executable(unit_test_aes tests/unit/test_crypto_aes.cpp)
target_include_directories(unit_test_aes PRIVATE src ${OPENSSL_INCLUDE_DIR})
target_link_libraries(unit_test_aes PRIVATE aes_asm_impl OpenSSL::SSL OpenSSL::Crypto)

add_executable(benchmark_aes tests/perf/benchmark_aes.cpp)
target_include_directories(benchmark_aes PRIVATE src ${OPENSSL_INCLUDE_DIR})
target_link_libraries(benchmark_aes PRIVATE aes_asm_impl OpenSSL::SSL OpenSSL::Crypto)

if(MSVC)
    target_compile_options(https_server PRIVATE /W4 /permissive-)
    target_compile_options(unit_test_aes PRIVATE /W4 /permissive-)
    target_compile_options(benchmark_aes PRIVATE /W4 /permissive-)
    if(CMAKE_BUILD_TYPE STREQUAL "Release")
        target_compile_options(https_server PRIVATE /O2 /DNDEBUG)
        target_compile_options(unit_test_aes PRIVATE /O2 /DNDEBUG)
        target_compile_options(benchmark_aes PRIVATE /O2 /DNDEBUG)
    endif()
else()
    target_compile_options(https_server PRIVATE -Wall -Wextra -Wpedantic -Wconversion)
    target_compile_options(unit_test_aes PRIVATE -Wall -Wextra -Wpedantic -Wconversion)
    target_compile_options(benchmark_aes PRIVATE -Wall -Wextra -Wpedantic -Wconversion)
    if(CMAKE_BUILD_TYPE STREQUAL "Debug")
        target_compile_options(https_server PRIVATE -g)
        target_compile_options(benchmark_aes PRIVATE -g)
        target_compile_options(unit_test_aes PRIVATE -g)
    elseif(CMAKE_BUILD_TYPE STREQUAL "Release")
        target_compile_options(https_server PRIVATE -O3 -DNDEBUG)
        target_compile_options(benchmark_aes PRIVATE -O3 -DNDEBUG)
        target_compile_options(unit_test_aes PRIVATE -O3 -DNDEBUG)
    endif()
endif()