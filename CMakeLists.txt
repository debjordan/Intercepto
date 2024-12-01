cmake_minimum_required(VERSION 3.10)
project(intercepto VERSION 1.0 LANGUAGES CXX)

# Define o padrão de compilação
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# Inclui os diretórios de cabeçalho
include_directories(include)

# Adiciona o executável principal
file(GLOB SOURCES "src/*.cpp")
add_executable(intercepto ${SOURCES})

# Linka as bibliotecas necessárias
find_package(Boost REQUIRED COMPONENTS system filesystem)
find_package(OpenSSL REQUIRED)
target_link_libraries(intercepto Boost::boost Boost::system Boost::filesystem OpenSSL::SSL)

# Adicione esta linha antes de find_package(Boost ...)
cmake_policy(SET CMP0167 NEW)

# Atualize a chamada do find_package
find_package(Boost REQUIRED COMPONENTS system filesystem)
include_directories(${Boost_INCLUDE_DIRS})
target_link_libraries(intercepto ${Boost_LIBRARIES})

