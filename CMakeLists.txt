cmake_minimum_required(VERSION 3.10)
project(Intercepto)

set(CMAKE_CXX_STANDARD 14)

find_package(Boost REQUIRED COMPONENTS system)

include_directories(${Boost_INCLUDE_DIRS})
include_directories(include)

add_executable(Intercepto src/main.cpp src/proxy.cpp)

target_link_libraries(Intercepto ${Boost_LIBRARIES})