cmake_minimum_required(VERSION 3.10)
project(Intercepto)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Boost REQUIRED COMPONENTS system)
find_package(Threads REQUIRED)

include_directories(${Boost_INCLUDE_DIRS})
include_directories(include)

add_executable(Intercepto src/main.cpp src/proxy.cpp)

target_link_libraries(Intercepto
    ${Boost_LIBRARIES}
    Threads::Threads
)
