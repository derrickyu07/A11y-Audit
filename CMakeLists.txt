cmake_minimum_required(VERSION 3.20)
project(a11y_audit LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_executable(a11y-audit src/main.cpp src/uia_tree.cpp)

target_compile_definitions(a11y-audit PRIVATE
  UNICODE _UNICODE          # use the wide (W) versions of Windows functions
  NOMINMAX                  # stop windows.h from defining min/max macros
)
target_link_libraries(a11y-audit PRIVATE UIAutomationCore ole32 oleaut32)

if (MSVC)
  target_compile_options(a11y-audit PRIVATE /W4)
endif()