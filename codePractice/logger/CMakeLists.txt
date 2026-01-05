cmake_minimum_required(VERSION 3.2)

project(logger)

add_executable(${PROJECT_NAME}
    src/fLog.cpp
    src/logHelper.cpp
    src/main.cpp
    src/logQueue.cpp
    src/asyncLog.cpp
)

target_include_directories(${PROJECT_NAME} PRIVATE
    ./include
)