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

Proxy::Proxy(const std::string& host, uint16_t port,
             ProxyCallback callback, std::size_t thread_count)
    : host_(host)
    , port_(port)
    , callback_(std::move(callback))
    , ioc_()
    , acceptor_(ioc_, {net::ip::make_address(host), port})
    , pool_(thread_count > 0 ? thread_count
                             : std::max(1u, std::thread::hardware_concurrency()))
{}

Proxy::~Proxy() {
    stop();
}

// --- Controle -----------------------------------------------------------------

void Proxy::start() {
    running_ = true;
    accept_thread_ = std::thread([this] { accept_loop(); });
}

void Proxy::stop() {
    if (!running_.exchange(false)) return;

    boost::system::error_code ec;
    acceptor_.close(ec);

    if (accept_thread_.joinable()) accept_thread_.join();

    pool_.stop();
    pool_.join();
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
        << bold << "  Porta         : " << reset << port_ << "\n"
        << bold << "  Tempo ativo   : " << reset << uptime << "s\n"
        << bold << "  Requisicoes   : " << reset << reqs << "\n"
        << bold << "  Dados entrada : " << reset
            << std::fixed << std::setprecision(1)
            << stats_.bytes_in.load() / 1024.0 << " KB\n"
        << bold << "  Dados saida   : " << reset
            << stats_.bytes_out.load() / 1024.0 << " KB\n";

    if (reqs > 0) {
        std::cout << bold << "  Latencia media: " << reset << lat / reqs << " ms\n";
    }

    std::cout << bold << "\n  Status codes:\n" << reset;
    {
        std::lock_guard<std::mutex> lk(stats_.mtx);
        for (const auto& [code, count] : stats_.status_codes) {
            std::cout << "    " << status_color(code) << code << reset
                      << "  ->  " << count << " req(s)\n";
        }
    }
    std::cout << "\n";
}

// --- Loop de aceitacao --------------------------------------------------------

void Proxy::accept_loop() {
    while (running_) {
        net::ip::tcp::socket socket{ioc_};
        boost::system::error_code ec;
        acceptor_.accept(socket, ec);

        if (ec) break;

        uint64_t id = ++req_counter_;
        net::post(pool_, [this, s = std::move(socket), id]() mutable {
            handle_request(std::move(s), id);
        });
    }
}

// --- Helpers ------------------------------------------------------------------

static std::pair<std::string, std::string> parse_host_header(const std::string& h) {
    auto colon  = h.rfind(':');
    auto colons = std::count(h.begin(), h.end(), ':');
    if (colon != std::string::npos && colons == 1) {
        return {h.substr(0, colon), h.substr(colon + 1)};
    }
    return {h, "80"};
}

static std::string format_id(uint64_t id) {
    char buf[8];
    std::snprintf(buf, sizeof(buf), "%04llu", static_cast<unsigned long long>(id));
    return buf;
}

static std::string truncate(const std::string& s, std::size_t max_len = 512) {
    if (s.size() <= max_len) return s;
    return s.substr(0, max_len) + " ... [" + std::to_string(s.size() - max_len) + " bytes omitidos]";
}

template<typename Message>
static std::string format_headers(const Message& msg) {
    std::string out;
    for (const auto& h : msg) {
        out += std::string(h.name_string()) + ": " + std::string(h.value()) + "\n";
    }
    return out;
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
        std::string method  = std::string(req.method_string());
        std::string id_str  = format_id(req_id);
        std::string ts      = timestamp();

        // -- Exibe no terminal -------------------------------------------------
        {
            std::lock_guard<std::mutex> lk(print_mutex());
            std::cout
                << gray  << "[" << ts << "] " << reset
                << magenta << "#" << id_str << reset
                << bold  << " -> REQUISICAO " << reset
                << gray  << "----------------------------" << reset << "\n"
                << "  " << bold << "Metodo  " << reset
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
            if (body.empty()) std::cout << gray << "[vazio]" << reset << "\n";
            else              std::cout << truncate(body) << "\n";
        }

        // -- Forward -----------------------------------------------------------
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

        // -- Exibe resposta no terminal ----------------------------------------
        {
            std::lock_guard<std::mutex> lk(print_mutex());
            std::cout
                << gray  << "[" << timestamp() << "] " << reset
                << magenta << "#" << id_str << reset
                << bold  << " <- RESPOSTA  " << reset
                << gray  << "(" << latency << "ms)  --------------------" << reset << "\n"
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
            if (body.empty()) std::cout << gray << "[vazio]" << reset << "\n";
            else              std::cout << truncate(body) << "\n";

            std::cout << gray << "----------------------------------------------------------" << reset << "\n\n";
        }

        stats_.record(status, req.body().size(), res.body().size(),
                      static_cast<uint64_t>(latency));

        // -- Emite evento para GUI (se callback configurado) -------------------
        if (callback_) {
            ProxyEvent ev;
            ev.id          = req_id;
            ev.port        = port_;
            ev.timestamp   = ts;
            ev.method      = method;
            ev.host        = target_host;
            ev.url         = std::string(req.target());
            ev.req_headers = format_headers(req);
            ev.req_body    = req.body();
            ev.status_code = status;
            ev.status_text = std::string(res.reason());
            ev.res_headers = format_headers(res);
            ev.res_body    = res.body();
            ev.latency_ms  = latency;
            callback_(std::move(ev));
        }

        // -- Devolve resposta ao cliente ---------------------------------------
        http::write(socket, res);

        beast::error_code ec;
        socket.shutdown(net::ip::tcp::socket::shutdown_both, ec);

    } catch (const std::exception& e) {
        std::lock_guard<std::mutex> lk(print_mutex());
        std::cout << color::bred << "[ERRO] #" << format_id(req_id)
                  << " -> " << e.what() << color::reset << "\n";

        if (callback_) {
            ProxyEvent ev;
            ev.id       = req_id;
            ev.port     = port_;
            ev.is_error = true;
            ev.error_msg = e.what();
            callback_(std::move(ev));
        }
    }
}
