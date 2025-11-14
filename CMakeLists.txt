
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

# Collect sources from src/ (recurses into subdirectories)
file(GLOB_RECURSE STS_SOURCES
	"${CMAKE_SOURCE_DIR}/src/*.cpp"
)

if(NOT STS_SOURCES)
	message(WARNING "No source files found in ${CMAKE_SOURCE_DIR}/src — add sources or adjust glob")
endif()

add_executable(STSSaveEditor ${STS_SOURCES})

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

# Helpful summary message
message(STATUS "Project: ${PROJECT_NAME} ${PROJECT_VERSION}")
message(STATUS "C Compiler: ${CMAKE_C_COMPILER}")
message(STATUS "CXX Compiler: ${CMAKE_CXX_COMPILER}")

