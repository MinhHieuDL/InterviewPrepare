cmake_minimum_required(VERSION 3.2)
project(stack)

# print build info
option(VERBOSE_BUILD_INFO "Show detailed build configuration in output" OFF)

include(../cmakeCommon/printBuildInfo.cmake)

if(VERBOSE_BUILD_INFO)
    print_build_info()
endif()

# main logic part
add_executable(${PROJECT_NAME}
    src/main.cpp
    src/stack.hpp
)

# target_include_directories(${PROJECT_NAME} PRIVATE
#     ./include
# )