#include "proxy.h"
#include <iostream>

namespace beast = boost::beast;
namespace http = beast::http;
namespace net = boost::asio;

void Proxy::start(const std::string& host, unsigned short port) {
    net::io_context ioc;
    net::ip::tcp::acceptor acceptor{ioc, {net::ip::make_address(host), port}};
    
    std::cout << "Proxy iniciado em " << host << ":" << port << std::endl;
    
    while (true) {
        net::ip::tcp::socket socket{ioc};
        acceptor.accept(socket);
        handle_request(socket);
    }
}

void Proxy::handle_request(net::ip::tcp::socket& socket) {
    try {
        beast::flat_buffer buffer;
        http::request<http::string_body> req;

        http::read(socket, buffer, req);

        std::cout << "Método: " << req.method_string() << "\n";
        std::cout << "URL: " << req.target() << "\n";
        for (const auto& header : req) {
            std::cout << header.name_string() << ": " << header.value() << "\n";
        }

        http::response<http::string_body> res{http::status::ok, req.version()};
        res.set(http::field::server, "Intercepto");
        res.body() = "Interceptado pelo Intercepto!";
        res.prepare_payload();
        http::write(socket, res);
    } catch (const std::exception& e) {
        std::cerr << "Erro ao lidar com requisição: " << e.what() << std::endl;
    }
}
