add_library(crypto INTERFACE)
target_include_directories(crypto INTERFACE ${CMAKE_CURRENT_LIST_DIR})

if(NOT TARGET Bootloader::crypto_impl)
    # add_library(sw_crypto INTERFACE EXCLUDE_FROM_ALL)
    # target_link_libraries(sw_crypto INTERFACE crypto Bootloader::config_base)

    if(BUILD_TESTING)
        add_executable(test_sw_crypto test_aes128.cpp)
        target_link_libraries(test_sw_crypto PRIVATE crypto Bootloader::test_base)
        add_test(NAME test_sw_crypto COMMAND test_sw_crypto)
    endif()

    add_library(Bootloader::crypto_impl ALIAS crypto)
endif()
