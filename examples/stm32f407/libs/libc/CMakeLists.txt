add_library(libc STATIC libc_functions.cpp)
target_compile_options(libc
    PRIVATE
        -fno-lto
        -Os
    PUBLIC
        -nodefaultlibs
)

target_link_options(libc
    PRIVATE
        -fno-lto
        -Os
    PUBLIC
        -nodefaultlibs
        --specs=nosys.specs
)