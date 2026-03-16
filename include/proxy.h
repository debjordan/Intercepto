#ifndef PROXY_H
#define PROXY_H

#include <boost/asio.hpp>
#include <boost/beast.hpp>

#include "proxy_event.h"

#include <atomic>
#include <chrono>
#include <cstdint>
#include <functional>
#include <map>
#include <memory>
#include <mutex>
#include <string>
#include <thread>

// Callback invocado pelas threads do proxy ao concluir cada requisicao.
// Pode ser null (modo CLI sem callback explícito).
using ProxyCallback = std::function<void(ProxyEvent)>;

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
        bytes_in         += b_in;
        bytes_out        += b_out;
        total_latency_ms += latency_ms;
        std::lock_guard<std::mutex> lk(mtx);
        status_codes[status]++;
    }
};

class Proxy {
public:
    // host/port: onde escutar. callback: notificado a cada requisicao concluida.
    Proxy(const std::string& host, uint16_t port,
          ProxyCallback callback = nullptr,
          std::size_t thread_count = 0);
    ~Proxy();

    // Inicia o loop de aceitacao em background (nao bloqueia).
    void start();

    // Sinaliza parada e aguarda todas as threads encerrarem.
    void stop();

    void print_stats() const;

    uint16_t    port()  const { return port_; }
    const ProxyStats& stats() const { return stats_; }

private:
    void accept_loop();
    void handle_request(boost::asio::ip::tcp::socket socket, uint64_t req_id);

    std::string                    host_;
    uint16_t                       port_;
    ProxyCallback                  callback_;
    boost::asio::io_context        ioc_;       // declarado antes de acceptor_
    boost::asio::ip::tcp::acceptor acceptor_;  // inicializado com ioc_
    boost::asio::thread_pool       pool_;
    std::atomic<bool>              running_{false};
    std::atomic<uint64_t>          req_counter_{0};
    std::thread                    accept_thread_;
    ProxyStats                     stats_;
};

#endif
