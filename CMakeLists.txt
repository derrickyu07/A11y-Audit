cmake_minimum_required(VERSION 3.20)
project(a11y_audit LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Rules and audit logic, with no COM calls, so tests can use it
add_library(a11y_core src/rules.cpp)
target_include_directories(a11y_core PUBLIC src)
target_compile_definitions(a11y_core PUBLIC UNICODE _UNICODE NOMINMAX)

# The command line tool
add_executable(a11y-audit src/main.cpp src/uia_tree.cpp)
target_link_libraries(a11y-audit PRIVATE a11y_core UIAutomationCore ole32 oleaut32)

if (MSVC)
  target_compile_options(a11y_core PRIVATE /W4)
  target_compile_options(a11y-audit PRIVATE /W4)
endif()

# Tests
include(FetchContent)
FetchContent_Declare(
  googletest
  URL https://github.com/google/googletest/archive/refs/tags/v1.15.2.zip
)
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)

enable_testing()
add_executable(a11y_tests tests/rules_test.cpp)
target_link_libraries(a11y_tests PRIVATE a11y_core GTest::gtest_main)
include(GoogleTest)
gtest_discover_tests(a11y_tests)