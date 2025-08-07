cmake_minimum_required(VERSION 3.2)
project(testStMachine)

# Print out the build info if have the request

option(VERBOSE_BUILD_INFO "Show detailed build configuration in output" OFF)

include(../cmakeCommon/printBuildInfo.cmake)

if(VERBOSE_BUILD_INFO)
    print_build_info()
endif()

# Main logic part 

add_executable(${PROJECT_NAME}
    src/fwStateMachine.cpp
    src/main.cpp
    src/test.cpp)

# Set include directories
target_include_directories(${PROJECT_NAME} PRIVATE
    include
)