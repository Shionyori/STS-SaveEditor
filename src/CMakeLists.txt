# ---- 编译 sts_codec 静态库 ----
add_library(sts_codec STATIC
    codec.cpp
    codec.h
)

target_include_directories(sts_codec PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
)

# ---- CLI 程序 ----
if(BUILD_CLI)
    add_executable(STSSaveEditor cli_main.cpp)
    target_link_libraries(STSSaveEditor PRIVATE sts_codec)

    install(TARGETS STSSaveEditor DESTINATION bin)
endif()
