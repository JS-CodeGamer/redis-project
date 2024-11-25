cmake_minimum_required(VERSION 3.10)

project(redis_mini)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

include_directories(include)

file(GLOB SOURCES "src/*.c")

add_executable(${PROJECT_NAME} ${SOURCES})

target_link_libraries(${PROJECT_NAME} m)

# target_compile_options(${PROJECT_NAME} PRIVATE -Wall -Wextra -pedantic)

