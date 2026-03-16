#pragma once

#include <cstdint>
#include <string>

// Representacao completa de um ciclo requisicao/resposta.
// Struct pura (sem Qt, sem Boost) para trafegar entre threads.
struct ProxyEvent {
    uint64_t    id{0};
    int         port{0};
    std::string timestamp;
    std::string method;
    std::string host;
    std::string url;
    std::string req_headers;
    std::string req_body;
    unsigned    status_code{0};
    std::string status_text;
    std::string res_headers;
    std::string res_body;
    int64_t     latency_ms{0};
    bool        is_error{false};
    std::string error_msg;
};
