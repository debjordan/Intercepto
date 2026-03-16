cmake_minimum_required(VERSION 3.16)
project(Intercepto VERSION 1.0.0)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Boost REQUIRED COMPONENTS system)
find_package(Threads REQUIRED)

include_directories(${Boost_INCLUDE_DIRS} include)

# ── Target CLI ────────────────────────────────────────────────────────────────
add_executable(intercepto
    src/main.cpp
    src/proxy.cpp
)
target_link_libraries(intercepto
    ${Boost_LIBRARIES}
    Threads::Threads
)

# ── Target GUI ────────────────────────────────────────────────────────────────
find_package(Qt6 COMPONENTS Core Widgets)

if(Qt6_FOUND)
    message(STATUS "Qt6 encontrado - compilando target intercepto-gui")

    # Gera moc_*.cpp explicitamente para os headers com Q_OBJECT
    qt6_wrap_cpp(GUI_MOC_FILES
        include/proxy_bridge.h
        include/proxy_manager.h
        include/mainwindow.h
    )

    set(GUI_SOURCES
        src/gui_main.cpp
        src/proxy.cpp
        src/proxy_bridge.cpp
        src/proxy_manager.cpp
        src/mainwindow.cpp
        ${GUI_MOC_FILES}
    )

    qt6_add_executable(intercepto-gui ${GUI_SOURCES})
    set_target_properties(intercepto-gui PROPERTIES
        AUTOUIC ON
        AUTORCC ON
    )
    target_include_directories(intercepto-gui PRIVATE ${Boost_INCLUDE_DIRS} include)
    target_link_libraries(intercepto-gui PRIVATE
        Qt6::Core
        Qt6::Widgets
        ${Boost_LIBRARIES}
        Threads::Threads
    )
else()
    message(WARNING "Qt6 nao encontrado - target intercepto-gui sera ignorado")
endif()

# ── Instalacao ────────────────────────────────────────────────────────────────
install(TARGETS intercepto RUNTIME DESTINATION bin)

if(Qt6_FOUND)
    install(TARGETS intercepto-gui RUNTIME DESTINATION bin)
endif()

# ── Pacote .deb via CPack ─────────────────────────────────────────────────────
set(CPACK_GENERATOR "DEB")
set(CPACK_PACKAGE_NAME "intercepto")
set(CPACK_PACKAGE_VERSION "${PROJECT_VERSION}")
set(CPACK_PACKAGE_CONTACT "jordan")
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "Proxy HTTP interceptor - CLI e GUI desktop")
set(CPACK_PACKAGE_DESCRIPTION
    "Intercepto e um proxy HTTP que captura e exibe requisicoes em tempo real.\n"
    "Inclui modo CLI (intercepto) e interface desktop Qt (intercepto-gui).")
set(CPACK_DEBIAN_PACKAGE_SECTION "net")
set(CPACK_DEBIAN_PACKAGE_ARCHITECTURE "amd64")
set(CPACK_DEBIAN_PACKAGE_DEPENDS
    "libboost-system1.74.0 (>= 1.74), libqt6widgets6 (>= 6.4), libstdc++6")
set(CPACK_DEBIAN_PACKAGE_MAINTAINER "jordan")

include(CPack)
