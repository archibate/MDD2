add_library(header-only OBJECT
        aes.cpp
        base64.cpp
)
target_include_directories(header-only PUBLIC include)
