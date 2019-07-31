cmake_minimum_required(VERSION 3.10)

# Project name

project(Quadtree VERSION 1.0 LANGUAGES CXX)

# Build type

set(CMAKE_BUILD_TYPE Release)

# Export compile commands (useful for editors)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Create library

add_library(quadtree INTERFACE)
target_include_directories(quadtree INTERFACE
    $<BUILD_INTERFACE:${CMAKE_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>)

# Set warnings

function(setWarnings target)
    target_compile_options(${target} PRIVATE
        -Wall
        -Wextra
        -Wshadow
        -Wnon-virtual-dtor
        -Wold-style-cast
        -Wcast-align
        -Wunused
        -Woverloaded-virtual
        -Wpedantic
        -Wconversion
        -Wsign-conversion
        -Wmisleading-indentation
        -Wduplicated-cond
        -Wduplicated-branches
        -Wlogical-op
        -Wnull-dereference
        -Wuseless-cast
        -Wdouble-promotion)
endfunction()

# Set standard

function(setStandard target)
    target_compile_features(${target} PRIVATE cxx_std_17)
endfunction()

# Benchmarks

option (BUILD_BENCHMARKS "Build the benchmarks." ON)
if (BUILD_BENCHMARKS)
    add_subdirectory(benchmarks)
endif()

# Examples

option (BUILD_EXAMPLES "Build the examples." ON)
if (BUILD_EXAMPLES)
    add_subdirectory(examples)
endif()

# Code coverage

option(CODE_COVERAGE "Enable coverage reporting" OFF)
if(CODE_COVERAGE)
    target_compile_options(quadtree INTERFACE
        -O0
        -g
        --coverage)
    target_link_libraries(quadtree INTERFACE --coverage)
endif()

# Tests

option (BUILD_TESTING "Build the testing tree." ON)
if (BUILD_TESTING)
    enable_testing()
    add_subdirectory(tests)
endif()