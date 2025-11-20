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
    add_executable(sts-editor cli_main.cpp)
    target_link_libraries(sts-editor PRIVATE sts_codec)

    install(TARGETS sts-editor DESTINATION bin)
endif()
