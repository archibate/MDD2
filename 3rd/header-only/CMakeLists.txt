add_library(3rd OBJECT
        aes.cpp
        base64.cpp
)
target_include_directories(3rd PUBLIC include)
