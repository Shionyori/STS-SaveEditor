cmake_minimum_required(VERSION 3.16)

project(STSSaveEditorGUI LANGUAGES CXX)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

set(BUILD_QT ON CACHE BOOL "Build with Qt GUI")

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
endif()

find_package(Qt6 REQUIRED COMPONENTS Widgets)

add_executable(STSSaveEditorGUI
    gui_main.cpp
    mainwindow.cpp
    mainwindow.h
)

target_link_libraries(STSSaveEditorGUI PRIVATE
    sts_codec
    Qt6::Widgets
)

target_include_directories(STSSaveEditorGUI PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/..
)

install(TARGETS STSSaveEditorGUI RUNTIME DESTINATION bin)

# ---- Windows: 自动拷贝 Qt DLL ----
if(WIN32)
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