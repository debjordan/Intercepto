#include <iostream>
#include <string>
#include <thread>
#include "proxy.h"
#include "capture.h"
#include "config.h"

void run_proxy(const std::string& host, unsigned short port) {
    intercepto::Proxy proxy;
    proxy.start(host, port);
}

void start_capture(const std::string& interface) {
    intercepto::Capture capture(interface, [](const std::vector<unsigned char>& packet) {
        std::cout << "Pacote capturado: " << packet.size() << " bytes" << std::endl;
    });

    capture.start_capture();
}

int main(int argc, char* argv[]) {
    if (argc != 2) {
        std::cerr << "Uso: " << argv[0] << " <arquivo_configuracao>" << std::endl;
        return 1;
    }

    // config carregadas apartir do json
    intercepto::Config config;
    if (!config.load(argv[1])) {
        std::cerr << "Erro ao carregar o arquivo de configuração!" << std::endl;
        return 1;
    }

    // interfaces e porta de dentro do arq de conf.
    std::vector<std::string> interfaces = config.get_interfaces();
    std::vector<int> ports = config.get_ports();

    std::cout << "Interfaces configuradas para captura:" << std::endl;
    for (const auto& interface : interfaces) {
        std::cout << "  " << interface << std::endl;
    }

    std::cout << "Portas configuradas para proxy:" << std::endl;
    for (const auto& port : ports) {
        std::cout << "  " << port << std::endl;
    }

    std::vector<std::thread> threads;

    for (const auto& interface : interfaces) {
        threads.push_back(std::thread(start_capture, interface));
    }

    for (const auto& port : ports) {
        threads.push_back(std::thread(run_proxy, "0.0.0.0", static_cast<unsigned short>(port)));
    }

    // Espera todas as threads terminarem (isso nunca vai acontecer com o código atual, porque está em loop infinito)
    for (auto& t : threads) {
        t.join();
    }

    return 0;
}
