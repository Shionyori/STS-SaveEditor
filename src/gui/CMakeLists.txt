cmake_minimum_required(VERSION 3.10)
project(STSSaveEditorGUI VERSION 1.0 LANGUAGES C CXX)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_C_STANDARD   11)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_C_STANDARD_REQUIRED   ON)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# -------------- 通用警告/优化 --------------
function(add_warnings target)
    if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
        target_compile_options(${target} PRIVATE
            -Wall -Wextra -Wpedantic -Wconversion -Wsign-conversion)
        if(CMAKE_BUILD_TYPE STREQUAL "Release")
            target_compile_options(${target} PRIVATE -O2)
        endif()
    elseif(MSVC)
        target_compile_options(${target} PRIVATE /W4)
    endif()
endfunction()


# -------------- 构建选项 --------------
if(BUILD_QT)
    # 1. 优先使用 QT_ROOT
    if(DEFINED ENV{QT_ROOT} AND NOT CMAKE_PREFIX_PATH)
        set(_qt_root $ENV{QT_ROOT})
        foreach(sub lib/cmake lib cmake)
            if(EXISTS "${_qt_root}/${sub}/Qt6")
                list(PREPEND CMAKE_PREFIX_PATH "${_qt_root}/${sub}")
                break()
            endif()
        endforeach()
    endif()

    find_package(Qt6 REQUIRED COMPONENTS Widgets)

    add_executable(STSSaveEditorGUI
        gui_main.cpp
        mainwindow.h
        mainwindow.cpp
        mainwindow.ui
        ../codec.cpp)

    target_include_directories(STSSaveEditorGUI PRIVATE src)
    target_link_libraries(STSSaveEditorGUI PRIVATE Qt6::Widgets)
    add_warnings(STSSaveEditorGUI)
    install(TARGETS STSSaveEditorGUI RUNTIME DESTINATION bin)

    # 2. Windows 自动拷贝 DLL & 插件
    if(WIN32 AND DEPLOY_QT_DLLS)
        set(dst $<TARGET_FILE_DIR:STSSaveEditorGUI>)
        add_custom_command(TARGET STSSaveEditorGUI POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E make_directory "${dst}/platforms"
            COMMAND ${CMAKE_COMMAND} -E copy_if_different
                $<TARGET_FILE:Qt6::Core>
                $<TARGET_FILE:Qt6::Gui>
                $<TARGET_FILE:Qt6::Widgets>
                "${dst}"
            COMMAND ${CMAKE_COMMAND} -E copy_if_different
                "$<TARGET_FILE_DIR:Qt6::Core>/../plugins/platforms/qwindows.dll"
                "${dst}/platforms"
            COMMENT "Deploying Qt DLLs and qwindows plugin"
        )
    endif()
    message(STATUS "Qt GUI enabled")
else()
    message(STATUS "Qt GUI disabled (use -DBUILD_QT=ON to enable)")
endif()

# -------------- 信息打印 --------------
message(STATUS "Build type: ${CMAKE_BUILD_TYPE}")
message(STATUS "C compiler: ${CMAKE_C_COMPILER}")
message(STATUS "CXX compiler: ${CMAKE_CXX_COMPILER}")