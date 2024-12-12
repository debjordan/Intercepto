#ifndef CAPTURE_H
#define CAPTURE_H

#include <pcap.h>
#include <string>
#include <vector>
#include <functional>

namespace intercepto {

class Capture {
public:
    Capture(const std::string& interface, std::function<void(const std::vector<unsigned char>&)> packet_callback);
    ~Capture();

    void start_capture();

private:
    // Função de callback que será chamada para processar cada pacote capturado
    static void packet_handler(unsigned char* user_data, const struct pcap_pkthdr* pkthdr, const unsigned char* packet);

    std::string interface_;                      
    pcap_t* handle_;                             
    std::function<void(const std::vector<unsigned char>&)> packet_callback_; 
};

}

#endif // CAPTURE_H
