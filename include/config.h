#ifndef CONFIG_H
#define CONFIG_H

#include <string>
#include <vector>
#include <nlohmann/json.hpp>

namespace intercepto {

class Config {
public:
    bool load(const std::string& file_path);

    std::vector<std::string> get_interfaces() const;
    std::vector<int> get_ports() const;

private:
    nlohmann::json config_data_; 

    bool parse_config(const nlohmann::json& config);
};

}

#endif // CONFIG_H
