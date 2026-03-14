#include <csignal>
#include <iostream>
#include "proxy.h"

// Ponteiro global para permitir acesso ao proxy no signal handler
static Proxy* g_proxy = nullptr;

static void on_signal(int /*sig*/) {
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

    std::string    host = argv[1];
    unsigned short port = static_cast<unsigned short>(std::stoi(argv[2]));

    Proxy proxy;
    g_proxy = &proxy;

    std::signal(SIGINT,  on_signal);
    std::signal(SIGTERM, on_signal);

    proxy.start(host, port);
    return 0;
}
