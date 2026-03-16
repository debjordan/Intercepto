#include <csignal>
#include <iostream>
#include <thread>
#include <chrono>
#include "proxy.h"
#include "logger.h"

static Proxy* g_proxy = nullptr;

static void on_signal(int) {
    std::cout << "\n";
    if (g_proxy) g_proxy->print_stats();
    std::exit(0);
}

int main(int argc, char* argv[]) {
    if (argc != 3) {
        std::cerr << "Uso: " << argv[0] << " <host> <porta>\n"
                  << "Ex : " << argv[0] << " 127.0.0.1 8080\n";
        return 1;
    }

    std::string host = argv[1];
    uint16_t    port = static_cast<uint16_t>(std::stoi(argv[2]));

    // Modo CLI: sem callback (proxy imprime direto no terminal)
    Proxy proxy(host, port);
    g_proxy = &proxy;

    std::signal(SIGINT,  on_signal);
    std::signal(SIGTERM, on_signal);

    proxy.start();

    using namespace color;
    std::cout
        << bold << cyan
        << "\n======================================================\n"
        << "       Intercepto - Proxy HTTP Interceptor           \n"
        << "======================================================"
        << reset << "\n"
        << bold << "  Endereco  : " << reset << cyan << host << ":" << port << reset << "\n"
        << bold << "  Threads   : " << reset << std::thread::hardware_concurrency() << "\n"
        << bold << "  Modo      : " << reset << "Transparent proxy (via Host header)\n"
        << gray  << "  Pressione Ctrl+C para encerrar\n"
        << reset << "\n";

    // Bloqueia ate SIGINT/SIGTERM (tratados por on_signal)
    while (true) {
        std::this_thread::sleep_for(std::chrono::seconds(1));
    }

    return 0;
}
