#ifndef PROXY_H
#define PROXY_H

#include <boost/asio.hpp>
#include <boost/beast.hpp>

#include <atomic>
#include <chrono>
#include <cstdint>
#include <map>
#include <mutex>
#include <string>
#include <thread>

// Estatísticas coletadas durante a execução do proxy
struct ProxyStats {
    std::atomic<uint64_t> total_requests{0};
    std::atomic<uint64_t> bytes_in{0};
    std::atomic<uint64_t> bytes_out{0};
    std::atomic<uint64_t> total_latency_ms{0};
    std::chrono::steady_clock::time_point start_time{std::chrono::steady_clock::now()};

    mutable std::mutex mtx;
    std::map<unsigned, uint64_t> status_codes;

    void record(unsigned status, uint64_t b_in, uint64_t b_out, uint64_t latency_ms) {
        total_requests++;
        bytes_in          += b_in;
        bytes_out         += b_out;
        total_latency_ms  += latency_ms;
        std::lock_guard<std::mutex> lk(mtx);
        status_codes[status]++;
    }
};

class Proxy {
public:
    // thread_count = 0 usa hardware_concurrency
    explicit Proxy(std::size_t thread_count = 0);
    ~Proxy();

    // Inicia o loop de aceitação na interface host:port
    void start(const std::string& host, unsigned short port);

    // Sinaliza parada; deve ser chamado de outro thread (ex.: signal handler)
    void stop();

    // Imprime sumário de estatísticas no stdout
    void print_stats() const;

private:
    void handle_request(boost::asio::ip::tcp::socket socket, uint64_t req_id);

    boost::asio::thread_pool pool_;
    std::atomic<bool>        running_{true};
    ProxyStats               stats_;
};

#endif
