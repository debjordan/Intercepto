#include "proxy.h"
#include "logger.h"

#include <algorithm>
#include <iomanip>
#include <iostream>
#include <sstream>

namespace beast = boost::beast;
namespace http  = beast::http;
namespace net   = boost::asio;

// --- Construcao / Destruicao ---------------------------------------------------

Proxy::Proxy(std::size_t thread_count)
    : pool_(thread_count > 0 ? thread_count
                             : std::max(1u, std::thread::hardware_concurrency())) {}

Proxy::~Proxy() {
    pool_.stop();
    pool_.join();
}

// --- Controle -----------------------------------------------------------------

void Proxy::stop() {
    running_ = false;
}

// --- Estatisticas -------------------------------------------------------------

void Proxy::print_stats() const {
    using namespace color;

    auto uptime = std::chrono::duration_cast<std::chrono::seconds>(
        std::chrono::steady_clock::now() - stats_.start_time).count();

    uint64_t reqs = stats_.total_requests.load();
    uint64_t lat  = stats_.total_latency_ms.load();

    std::cout
        << "\n" << bold << cyan
        << "==========================================\n"
        << "         Estatisticas Finais              \n"
        << "=========================================="
        << reset << "\n"
        << bold << "  Tempo ativo   : " << reset << uptime << "s\n"
        << bold << "  Requisições   : " << reset << reqs << "\n"
        << bold << "  Dados recebidos: " << reset
            << std::fixed << std::setprecision(1)
            << stats_.bytes_in.load() / 1024.0 << " KB\n"
        << bold << "  Dados enviados : " << reset
            << stats_.bytes_out.load() / 1024.0 << " KB\n";

    if (reqs > 0) {
        std::cout << bold << "  Latência média : " << reset << lat / reqs << " ms\n";
    }

    std::cout << bold << "\n  Distribuição de status:\n" << reset;
    {
        std::lock_guard<std::mutex> lk(stats_.mtx);
        for (const auto& [code, count] : stats_.status_codes) {
            std::cout << "    "
                      << status_color(code) << code << reset
                      << "  ->  " << count << " req(s)\n";
        }
    }
    std::cout << "\n";
}

// --- Loop de aceitacao --------------------------------------------------------

void Proxy::start(const std::string& host, unsigned short port) {
    using namespace color;

    net::io_context ioc;
    net::ip::tcp::acceptor acceptor{ioc, {net::ip::make_address(host), port}};

    std::cout
        << bold << cyan
        << "\n======================================================\n"
        << "       Intercepto - Proxy HTTP Interceptor           \n"
        << "======================================================"
        << reset << "\n"
        << bold << "  Endereço  : " << reset << cyan << host << ":" << port << reset << "\n"
        << bold << "  Threads   : " << reset << std::thread::hardware_concurrency() << "\n"
        << bold << "  Modo      : " << reset << "Transparent proxy (via Host header)\n"
        << gray  << "  Pressione Ctrl+C para encerrar e ver estatísticas\n"
        << reset << "\n";

    std::atomic<uint64_t> req_counter{0};

    while (running_) {
        net::ip::tcp::socket socket{ioc};
        boost::system::error_code ec;
        acceptor.accept(socket, ec);

        if (ec) {
            if (!running_) break;
            continue;   // erro transitório, volta a escutar
        }

        uint64_t id = ++req_counter;
        net::post(pool_, [this, s = std::move(socket), id]() mutable {
            handle_request(std::move(s), id);
        });
    }

    pool_.join();
}

// --- Helpers ------------------------------------------------------------------

// Separa "host" e "porta" do valor do header Host (ex.: "api.foo.com:8080")
static std::pair<std::string, std::string> parse_host_header(const std::string& host_hdr) {
    // Evita confundir endereços IPv6 (que têm múltiplos ':')
    auto colon = host_hdr.rfind(':');
    if (colon != std::string::npos) {
        auto colons = std::count(host_hdr.begin(), host_hdr.end(), ':');
        if (colons == 1) {
            return {host_hdr.substr(0, colon), host_hdr.substr(colon + 1)};
        }
    }
    return {host_hdr, "80"};
}

static std::string format_id(uint64_t id) {
    char buf[8];
    std::snprintf(buf, sizeof(buf), "%04llu", static_cast<unsigned long long>(id));
    return buf;
}

static std::string truncate(const std::string& s, std::size_t max_len = 512) {
    if (s.size() <= max_len) return s;
    return s.substr(0, max_len) + color::gray + std::string(" … [") +
           std::to_string(s.size() - max_len) + " bytes omitidos]" + color::reset;
}

// --- Handler de requisicao ----------------------------------------------------

void Proxy::handle_request(net::ip::tcp::socket socket, uint64_t req_id) {
    using namespace color;

    try {
        auto t_start = std::chrono::steady_clock::now();

        beast::flat_buffer buf;
        http::request<http::string_body> req;
        http::read(socket, buf, req);

        std::string host_hdr = std::string(req[http::field::host]);
        auto [target_host, target_port] = parse_host_header(host_hdr);
        std::string method = std::string(req.method_string());
        std::string id_str = format_id(req_id);

        // -- Exibe requisicao --------------------------------------------------
        {
            std::lock_guard<std::mutex> lk(print_mutex());
            std::cout
                << gray  << "[" << timestamp() << "] " << reset
                << magenta << "#" << id_str << reset
                << bold  << " -> REQUISICAO " << reset
                << gray  << "----------------------------" << reset << "\n"
                << "  " << bold << "Método  " << reset
                << method_color(method) << method << reset
                << gray << "  ->  " << reset
                << cyan << target_host << ":" << target_port << reset << "\n"
                << "  " << bold << "URL     " << reset << req.target() << "\n"
                << "  " << bold << "Headers " << reset;

            bool first = true;
            for (const auto& h : req) {
                if (!first) std::cout << "          ";
                std::cout << gray << h.name_string() << ": " << reset << h.value() << "\n";
                first = false;
            }

            const auto& body = req.body();
            std::cout << "  " << bold << "Payload " << reset;
            if (body.empty()) {
                std::cout << gray << "[vazio]" << reset << "\n";
            } else {
                std::cout << truncate(body) << "\n";
            }
        }

        // -- Encaminha para o destino real (extraido do Host header) -----------
        http::request<http::string_body> fwd = req;
        fwd.set(http::field::host, target_host);

        net::io_context ioc;
        net::ip::tcp::resolver resolver{ioc};
        auto endpoints = resolver.resolve(target_host, target_port);

        beast::tcp_stream stream{ioc};
        stream.connect(endpoints);
        http::write(stream, fwd);

        beast::flat_buffer res_buf;
        http::response<http::string_body> res;
        http::read(stream, res_buf, res);

        auto latency = std::chrono::duration_cast<std::chrono::milliseconds>(
            std::chrono::steady_clock::now() - t_start).count();

        unsigned status = res.result_int();

        // -- Exibe resposta ----------------------------------------------------
        {
            std::lock_guard<std::mutex> lk(print_mutex());
            std::cout
                << gray  << "[" << timestamp() << "] " << reset
                << magenta << "#" << id_str << reset
                << bold  << " <- RESPOSTA  " << reset
                << gray  << "(" << latency << "ms)  "
                << "--------------------" << reset << "\n"
                << "  " << bold << "Status  " << reset
                << status_color(status) << status << " " << res.reason() << reset << "\n"
                << "  " << bold << "Headers " << reset;

            bool first = true;
            for (const auto& h : res) {
                if (!first) std::cout << "          ";
                std::cout << gray << h.name_string() << ": " << reset << h.value() << "\n";
                first = false;
            }

            const auto& body = res.body();
            std::cout << "  " << bold << "Payload " << reset;
            if (body.empty()) {
                std::cout << gray << "[vazio]" << reset << "\n";
            } else {
                std::cout << truncate(body) << "\n";
            }

            std::cout << gray << "----------------------------------------------------------" << reset << "\n\n";
        }

        stats_.record(status, req.body().size(), res.body().size(),
                      static_cast<uint64_t>(latency));

        // Devolve a resposta ao cliente original
        http::write(socket, res);

        beast::error_code ec;
        socket.shutdown(net::ip::tcp::socket::shutdown_both, ec);

    } catch (const std::exception& e) {
        std::lock_guard<std::mutex> lk(print_mutex());
        std::cout << color::bred << "[ERRO] #" << format_id(req_id)
                  << " -> " << e.what() << color::reset << "\n";
    }
}
