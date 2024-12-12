#include "capture.h"
#include <pcap.h>
#include <iostream>

namespace intercepto {

Capture::Capture(const std::string& interface, std::function<void(const std::vector<unsigned char>&)> packet_callback)
    : interface_(interface), packet_callback_(packet_callback) {
    char errbuf[PCAP_ERRBUF_SIZE];

    handle_ = pcap_open_live(interface.c_str(), BUFSIZ, 1, 1000, errbuf);
    if (!handle_) {
        std::cerr << "Não foi possível abrir a interface para captura: " << errbuf << std::endl;
        throw std::runtime_error("Erro ao abrir interface de captura");
    }
}

Capture::~Capture() {
    if (handle_) {
        pcap_close(handle_);
    }
}

void Capture::start_capture() {
    pcap_loop(handle_, 0, packet_handler, reinterpret_cast<unsigned char*>(this));
}

void Capture::packet_handler(unsigned char* user_data, const struct pcap_pkthdr* pkthdr, const unsigned char* packet) {
    Capture* capture = reinterpret_cast<Capture*>(user_data);
    std::vector<unsigned char> packet_data(packet, packet + pkthdr->len);
    capture->packet_callback_(packet_data);
}
}
