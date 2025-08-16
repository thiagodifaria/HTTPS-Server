add_library(aes_provider SHARED aes_provider.cpp)

target_link_libraries(aes_provider PRIVATE aes_asm_impl sha256_asm_impl)
target_link_libraries(aes_provider PRIVATE OpenSSL::SSL OpenSSL::Crypto)
target_include_directories(aes_provider PRIVATE .. ${OPENSSL_INCLUDE_DIR})

if(WIN32)
    target_link_options(aes_provider PRIVATE /LARGEADDRESSAWARE)
endif()

add_custom_command(TARGET aes_provider POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
    $<TARGET_FILE:aes_provider>
    $<TARGET_FILE_DIR:https_server>
)

if(MSVC)
    target_compile_options(aes_provider PRIVATE /W4 /permissive-)
else()
    target_compile_options(aes_provider PRIVATE -Wall -Wextra -Wpedantic -Wconversion)
endif()