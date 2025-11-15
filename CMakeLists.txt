
cmake_minimum_required(VERSION 3.10)

project(STSSaveEditor VERSION 1.0 LANGUAGES C CXX)

# Default to Release if no build type was specified
if(NOT CMAKE_BUILD_TYPE)
	set(CMAKE_BUILD_TYPE "Release" CACHE STRING "Build type" FORCE)
endif()

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Enable AUTOMOC so Q_OBJECT classes have moc generated automatically
set(CMAKE_AUTOMOC ON)

# Collect sources from src/ (recurses into subdirectories)
# Option to build Qt GUI (set ON to attempt building GUI when Qt is available)
option(BUILD_QT "Build the Qt GUI (requires Qt6 Widgets)" ON)
option(DEPLOY_QT_DLLS "Automatically copy Qt runtime DLLs and plugins into the GUI build folder on Windows" ON)
# Console executable (keeps original behavior)
set(CONSOLE_SOURCES
	${CMAKE_SOURCE_DIR}/src/cli/main.cpp
	${CMAKE_SOURCE_DIR}/src/codec.cpp
)

add_executable(STSSaveEditor ${CONSOLE_SOURCES})
target_include_directories(STSSaveEditor PRIVATE ${CMAKE_SOURCE_DIR}/src)

# Compiler-specific flags: prefer useful warnings for GCC/Clang, and /W4 for MSVC
if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
	target_compile_options(STSSaveEditor PRIVATE -Wall -Wextra -Wpedantic -Wconversion -Wsign-conversion)
	# Enable extra optimizations for release builds when using GCC
	if(CMAKE_BUILD_TYPE STREQUAL "Release")
		target_compile_options(STSSaveEditor PRIVATE -O2)
	endif()
elseif(MSVC)
	target_compile_options(STSSaveEditor PRIVATE /W4)
endif()

# Install target (optional)
install(TARGETS STSSaveEditor RUNTIME DESTINATION bin)

if(BUILD_QT)
	# If the user set QT_ROOT environment variable, prefer it to locate Qt6 CMake files.
	# This allows users to set QT_ROOT=D:/Qt/6.8.3/mingw_64 and CMake will try
	# D:/Qt/6.8.3/mingw_64/lib/cmake or related subfolders automatically.
	if(NOT DEFINED Qt6_DIR)
		if(NOT CMAKE_PREFIX_PATH)
			if(DEFINED ENV{QT_ROOT})
				set(_qt_root $ENV{QT_ROOT})
				message(STATUS "QT_ROOT environment variable detected: ${_qt_root}")
				# Prefer the typical Qt CMake folder locations
				if(EXISTS "${_qt_root}/lib/cmake/Qt6")
					list(APPEND CMAKE_PREFIX_PATH "${_qt_root}/lib/cmake")
				elseif(EXISTS "${_qt_root}/lib/cmake")
					list(APPEND CMAKE_PREFIX_PATH "${_qt_root}/lib/cmake")
				elseif(EXISTS "${_qt_root}/cmake/Qt6")
					list(APPEND CMAKE_PREFIX_PATH "${_qt_root}/cmake")
				elseif(EXISTS "${_qt_root}/cmake")
					list(APPEND CMAKE_PREFIX_PATH "${_qt_root}/cmake")
				else()
					# Fallback: add the root itself
					list(APPEND CMAKE_PREFIX_PATH "${_qt_root}")
				endif()
				message(STATUS "CMAKE_PREFIX_PATH set to: ${CMAKE_PREFIX_PATH}")
			endif()
		endif()
	endif()

	find_package(Qt6 REQUIRED COMPONENTS Widgets)
	set(GUI_SOURCES
		${CMAKE_SOURCE_DIR}/src/gui_main.cpp
		${CMAKE_SOURCE_DIR}/src/codec.cpp
	)
	add_executable(STSSaveEditorGUI ${GUI_SOURCES})
	target_include_directories(STSSaveEditorGUI PRIVATE ${CMAKE_SOURCE_DIR}/src)
	target_link_libraries(STSSaveEditorGUI PRIVATE Qt6::Widgets)
	install(TARGETS STSSaveEditorGUI RUNTIME DESTINATION bin)

	# Optionally copy Qt runtime DLLs and minimal plugin set into the GUI exe folder (Windows)
	if(WIN32 AND DEPLOY_QT_DLLS)
		# Ensure destination platforms directory exists
		add_custom_command(TARGET STSSaveEditorGUI POST_BUILD
			COMMAND ${CMAKE_COMMAND} -E make_directory "$<TARGET_FILE_DIR:STSSaveEditorGUI>/platforms"
		)

		# Copy core Qt DLLs (Core, Gui, Widgets) if targets exist
		# Use generator-expressions to reference imported Qt targets
		add_custom_command(TARGET STSSaveEditorGUI POST_BUILD
			COMMAND ${CMAKE_COMMAND} -E copy_if_different $<TARGET_FILE:Qt6::Core> $<TARGET_FILE_DIR:STSSaveEditorGUI>
			COMMAND ${CMAKE_COMMAND} -E copy_if_different $<TARGET_FILE:Qt6::Gui> $<TARGET_FILE_DIR:STSSaveEditorGUI>
			COMMAND ${CMAKE_COMMAND} -E copy_if_different $<TARGET_FILE:Qt6::Widgets> $<TARGET_FILE_DIR:STSSaveEditorGUI>
			# Copy the platform plugin (qwindows.dll) from the Qt install relative layout
			COMMAND ${CMAKE_COMMAND} -E copy_if_different "$<TARGET_FILE_DIR:Qt6::Core>/../plugins/platforms/qwindows.dll" "$<TARGET_FILE_DIR:STSSaveEditorGUI>/platforms/qwindows.dll"
			COMMENT "Copying Qt runtime DLLs and qwindows plugin into GUI output folder"
		)
	endif()

	message(STATUS "Qt GUI target enabled (STSSaveEditorGUI)")
else()
	message(STATUS "Qt GUI not built. Set -DBUILD_QT=ON and provide CMAKE_PREFIX_PATH to Qt6 if you want it.")
endif()

# Helpful summary message
message(STATUS "Project: ${PROJECT_NAME} ${PROJECT_VERSION}")
message(STATUS "C Compiler: ${CMAKE_C_COMPILER}")
message(STATUS "CXX Compiler: ${CMAKE_CXX_COMPILER}")

