cmake_minimum_required(VERSION 3.10)
project(intercepto VERSION 1.0 LANGUAGES CXX)

# Defina o caminho para o vcpkg (ajuste o caminho conforme necessário)
set(CMAKE_TOOLCHAIN_FILE "C:/Program Files/vcpkg/scripts/buildsystems/vcpkg.cmake" CACHE STRING "Vcpkg toolchain file" FORCE)

# Defina o caminho para a instalação do vcpkg
set(CMAKE_PREFIX_PATH "C:/Program Files/vcpkg/installed/x64-windows/share" CACHE PATH "Vcpkg prefix path")

# Defina o padrão de compilação
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# Encontre o pacote nlohmann-json
find_package(nlohmann_json CONFIG REQUIRED)

# Inclui os diretórios de cabeçalho
include_directories(include)

# Adiciona o diretório de fontes
file(GLOB SOURCES "src/*.cpp")

# Adiciona o executável principal
add_executable(intercepto ${SOURCES})

# Linka as bibliotecas necessárias
find_package(Boost REQUIRED COMPONENTS system filesystem)
find_package(OpenSSL REQUIRED)
find_package(PkgConfig REQUIRED)

# Linka libpcap (se necessário, dependendo da instalação)
pkg_check_modules(PCAP REQUIRED libpcap)

# Inclui diretórios necessários
target_include_directories(intercepto PRIVATE ${Boost_INCLUDE_DIRS} ${OPENSSL_INCLUDE_DIR} ${PCAP_INCLUDE_DIRS})

# Linka as bibliotecas
target_link_libraries(intercepto Boost::system Boost::filesystem OpenSSL::SSL OpenSSL::Crypto ${PCAP_LIBRARIES} nlohmann_json::nlohmann_json)
