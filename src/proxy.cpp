#include "proxy.h"
#include <boost/asio.hpp>
#include <boost/beast.hpp>
#include <iostream>

namespace intercepto {
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

        if (!req.body().empty()) {
            std::cout << "Body da requisição: " << req.body() << "\n";
        }

        forward_request(socket, req);

    } catch (const std::exception& e) {
        std::cerr << "Erro ao lidar com requisição: " << e.what() << std::endl;
    }
}

void Proxy::forward_request(net::ip::tcp::socket& socket, http::request<http::string_body>& req) {
    try {
        net::io_context ioc;
        net::ip::tcp::resolver resolver(ioc);
        auto const results = resolver.resolve("localhost", "80");
        net::ip::tcp::socket server_socket(ioc);
        net::connect(server_socket, results.begin(), results.end());

        http::write(server_socket, req);

        // Recebe a resposta do servidor
        beast::flat_buffer buffer;
        http::response<http::string_body> res;
        http::read(server_socket, buffer, res);

        std::cout << "Status da resposta: " << res.result_int() << "\n";
        for (const auto& header : res) {
            std::cout << header.name_string() << ": " << header.value() << "\n";
        }

        if (!res.body().empty()) {
            std::cout << "Corpo da resposta: " << res.body() << "\n";
        }

        send_response(socket, res);

    } catch (const std::exception& e) {
        std::cerr << "Erro ao encaminhar requisição: " << e.what() << std::endl;
    }
}

void Proxy::send_response(net::ip::tcp::socket& socket, http::response<http::string_body>& res) {
    try {
        http::write(socket, res);
    } catch (const std::exception& e) {
        std::cerr << "Erro ao enviar resposta: " << e.what() << std::endl;
    }
}

}
