cmake_minimum_required(VERSION 3.16)

project(STSSaveEditorGUI LANGUAGES CXX)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

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
