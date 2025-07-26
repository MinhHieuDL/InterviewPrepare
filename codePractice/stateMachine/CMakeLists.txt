cmake_minimum_required(VERSION 3.2)
project(testStMachine)

add_executable(${PROJECT_NAME}
    src/fwStateMachine.cpp
    src/main.cpp
    src/test.cpp)

# Set include directories
target_include_directories(${PROJECT_NAME} PRIVATE
    include
)