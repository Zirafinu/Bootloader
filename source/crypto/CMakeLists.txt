add_library(crypto INTERFACE)
target_include_directories(crypto INTERFACE ${CMAKE_CURRENT_LIST_DIR})
add_library(Bootloader::crypto_impl ALIAS crypto)

if(BUILD_TESTING)
    add_executable(test_sw_crypto test_aes128.cpp)
    target_link_libraries(test_sw_crypto PRIVATE crypto)
    add_test(NAME test_sw_crypto COMMAND test_sw_crypto)
endif()
