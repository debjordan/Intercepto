#include "config.h"
#include <fstream>
#include <nlohmann/json.hpp>
#include <iostream>

namespace intercepto {

bool Config::load(const std::string& file_path) {
    std::ifstream file(file_path);
    if (!file.is_open()) {
        std::cerr << "Não foi possível abrir o arquivo de configuração: " << file_path << std::endl;
        return false;
    }

    try {
        file >> config_data_;
        return parse_config(config_data_);
    } catch (const std::exception& e) {
        std::cerr << "Erro ao carregar as configurações: " << e.what() << std::endl;
        return false;
    }
}

bool Config::parse_config(const nlohmann::json& config) {
    try {
        if (config.contains("interfaces")) {
            for (const auto& interface : config["interfaces"]) {
                std::cout << "Interface: " << interface.get<std::string>() << std::endl;
            }
        }

        if (config.contains("ports")) {
            for (const auto& port : config["ports"]) {
                std::cout << "Porta: " << port.get<int>() << std::endl;
            }
        }

        return true;
    } catch (const std::exception& e) {
        std::cerr << "Erro ao processar configurações: " << e.what() << std::endl;
        return false;
    }
}

std::vector<std::string> Config::get_interfaces() const {
    std::vector<std::string> interfaces;
    if (config_data_.contains("interfaces")) {
        for (const auto& interface : config_data_["interfaces"]) {
            interfaces.push_back(interface.get<std::string>());
        }
    }
    return interfaces;
}

std::vector<int> Config::get_ports() const {
    std::vector<int> ports;
    if (config_data_.contains("ports")) {
        for (const auto& port : config_data_["ports"]) {
            ports.push_back(port.get<int>());
        }
    }
    return ports;
}

} // namespace intercepto
