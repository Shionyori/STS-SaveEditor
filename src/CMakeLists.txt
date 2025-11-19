add_library(sts_codec STATIC
    codec.h
    codec.cpp
)
target_include_directories(sts_codec PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

add_executable(sts-editor cli_main.cpp)
target_link_libraries(sts-editor PRIVATE sts_codec)

install(TARGETS sts-editor DESTINATION bin)