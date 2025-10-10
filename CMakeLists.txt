cmake_minimum_required(VERSION 3.20)
project(BuddyAllocator VERSION 1.0.0 LANGUAGES CXX)

# Set C++17 standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Export compile commands for clang-tidy
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Options
option(BUILD_TESTS "Build test executables" ON)
option(BUILD_BENCHMARKS "Build benchmark executables" ON)
option(ENABLE_SANITIZERS "Enable sanitizers (ASan/UBSan)" OFF)

# Include FetchContent for dependency management
include(FetchContent)

# Fetch toml11 (header-only TOML parser)
FetchContent_Declare(
    toml11
    GIT_REPOSITORY https://github.com/ToruNiina/toml11.git
    GIT_TAG v3.8.1
)
FetchContent_MakeAvailable(toml11)

# Fetch GoogleTest if tests are enabled
if(BUILD_TESTS)
    FetchContent_Declare(
        googletest
        GIT_REPOSITORY https://github.com/google/googletest.git
        GIT_TAG v1.14.0
    )
    # For Windows: Prevent overriding the parent project's compiler/linker settings
    set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
    FetchContent_MakeAvailable(googletest)
    enable_testing()
endif()

# Fetch Google Benchmark if benchmarks are enabled
if(BUILD_BENCHMARKS)
    set(BENCHMARK_ENABLE_TESTING OFF CACHE BOOL "Disable benchmark self-tests" FORCE)
    set(BENCHMARK_ENABLE_GTEST_TESTS OFF CACHE BOOL "Disable benchmark gtest tests" FORCE)
    FetchContent_Declare(
        benchmark
        GIT_REPOSITORY https://github.com/google/benchmark.git
        GIT_TAG v1.8.3
    )
    FetchContent_MakeAvailable(benchmark)
endif()

# Compiler warnings
if(MSVC)
    add_compile_options(/W4 /WX)
else()
    add_compile_options(-Wall -Wextra -Wpedantic)
    # Don't treat warnings as errors by default (can be enabled in CI)
    # add_compile_options(-Werror)
endif()

# Sanitizers
if(ENABLE_SANITIZERS)
    if(CMAKE_CXX_COMPILER_ID MATCHES "Clang|GNU")
        add_compile_options(-fsanitize=address,undefined -fno-omit-frame-pointer)
        add_link_options(-fsanitize=address,undefined)
    endif()
endif()

# =============================================================================
# Library: Custom Allocator
# =============================================================================
add_library(custom_allocator STATIC
    src/allocator/custom_allocator.cpp
    src/allocator/custom_allocator.h
)
target_include_directories(custom_allocator PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/src/allocator
)

# =============================================================================
# Library: Data Logger
# =============================================================================
add_library(data_logger STATIC
    src/logger/data_logger.cpp
    src/logger/data_logger.h
)
target_include_directories(data_logger PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/src/logger
)

# =============================================================================
# Library: Config Manager
# =============================================================================
add_library(config_manager STATIC
    src/config/config_manager.cpp
    src/config/config_manager.h
)
target_include_directories(config_manager PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/src/config
    ${CMAKE_CURRENT_SOURCE_DIR}  # For cxxopts.hpp
)
target_link_libraries(config_manager PUBLIC toml11::toml11)

# =============================================================================
# Executable: Unit Tests
# =============================================================================
if(BUILD_TESTS)
    add_executable(unit_tests
        src/tests/unit_tests.cpp
    )
    target_link_libraries(unit_tests PRIVATE
        custom_allocator
        GTest::gtest
        GTest::gtest_main
    )
    target_include_directories(unit_tests PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/src/allocator
    )
    
    # Register with CTest
    include(GoogleTest)
    gtest_discover_tests(unit_tests)
endif()

# =============================================================================
# Executable: Allocator Tests
# =============================================================================
if(BUILD_TESTS)
    add_executable(allocator_tests
        src/tests/allocator_tests.cpp
    )
    target_link_libraries(allocator_tests PRIVATE
        custom_allocator
        data_logger
        config_manager
    )
    target_include_directories(allocator_tests PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/src/allocator
        ${CMAKE_CURRENT_SOURCE_DIR}/src/logger
        ${CMAKE_CURRENT_SOURCE_DIR}/src/config
        ${CMAKE_CURRENT_SOURCE_DIR}  # For cxxopts.hpp
    )
endif()

# =============================================================================
# Executable: Performance Tests
# =============================================================================
if(BUILD_TESTS)
    add_executable(performance_tests
        src/tests/performance_tests.cpp
    )
    target_link_libraries(performance_tests PRIVATE
        custom_allocator
        data_logger
        config_manager
    )
    target_include_directories(performance_tests PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/src/allocator
        ${CMAKE_CURRENT_SOURCE_DIR}/src/logger
        ${CMAKE_CURRENT_SOURCE_DIR}/src/config
        ${CMAKE_CURRENT_SOURCE_DIR}  # For cxxopts.hpp
    )
endif()

# =============================================================================
# Executable: Stress Test (Google Benchmark)
# =============================================================================
if(BUILD_BENCHMARKS)
    add_executable(stress_test
        src/tests/stress_test.cpp
    )
    target_link_libraries(stress_test PRIVATE
        custom_allocator
        data_logger
        config_manager
        benchmark::benchmark
    )
    target_include_directories(stress_test PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/src/allocator
        ${CMAKE_CURRENT_SOURCE_DIR}/src/logger
        ${CMAKE_CURRENT_SOURCE_DIR}/src/config
        ${CMAKE_CURRENT_SOURCE_DIR}  # For cxxopts.hpp
    )
endif()

# =============================================================================
# Installation
# =============================================================================
install(TARGETS custom_allocator data_logger config_manager
    ARCHIVE DESTINATION lib
    LIBRARY DESTINATION lib
)

install(FILES
    src/allocator/custom_allocator.h
    src/logger/data_logger.h
    src/config/config_manager.h
    DESTINATION include
)

if(BUILD_TESTS)
    install(TARGETS allocator_tests performance_tests unit_tests
        RUNTIME DESTINATION bin
    )
endif()

if(BUILD_BENCHMARKS)
    install(TARGETS stress_test
        RUNTIME DESTINATION bin
    )
endif()

# Install config file
install(FILES config/default.toml
    DESTINATION config
)

# =============================================================================
# Summary
# =============================================================================
message(STATUS "")
message(STATUS "BuddyAllocator Configuration Summary:")
message(STATUS "  CMake version      : ${CMAKE_VERSION}")
message(STATUS "  C++ standard       : C++${CMAKE_CXX_STANDARD}")
message(STATUS "  Build type         : ${CMAKE_BUILD_TYPE}")
message(STATUS "  Build tests        : ${BUILD_TESTS}")
message(STATUS "  Build benchmarks   : ${BUILD_BENCHMARKS}")
message(STATUS "  Enable sanitizers  : ${ENABLE_SANITIZERS}")
message(STATUS "  Compiler           : ${CMAKE_CXX_COMPILER_ID} ${CMAKE_CXX_COMPILER_VERSION}")
message(STATUS "")

