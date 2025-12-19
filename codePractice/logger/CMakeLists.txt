cmake_minimum_required(VERSION 3.2)

project(logger)

add_executable(${PROJECT_NAME}
    src/fLog.cpp
    src/main.cpp
)

target_include_directories(${PROJECT_NAME} PRIVATE
    ./include
)