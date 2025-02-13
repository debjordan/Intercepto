#include "proxy.h"
#include <iostream>
#include <thread>
#include <vector>

namespace beast = boost::beast;
namespace http = beast::http;
namespace net = boost::asio;

void Proxy::start(const std::string &host, unsigned short port)
{
    net::io_context ioc;
    net::ip::tcp::acceptor acceptor{ioc, {net::ip::make_address(host), port}};

    std::cout << "Proxy iniciado em " << host << ":" << port << std::endl;

    while (true)
    {
        net::ip::tcp::socket socket{ioc};
        acceptor.accept(socket);
        std::thread(&Proxy::handle_request, this, std::move(socket)).detach();
    }
}

void Proxy::handle_request(net::ip::tcp::socket socket)
{
    try
    {
        beast::flat_buffer buffer;
        http::request<http::string_body> req;

        http::read(socket, buffer, req);

        std::cout << "=== Requisição Recebida ===\n";
        std::cout << "Método: " << req.method_string() << "\n";
        std::cout << "URL: " << req.target() << "\n";
        std::cout << "Headers:\n";
        for (const auto &header : req)
        {
            std::cout << "  " << header.name_string() << ": " << header.value() << "\n";
        }

        if (!req.body().empty())
        {
            std::cout << "Payload: " << req.body() << "\n";
        }
        else
        {
            std::cout << "Payload: [Vazio]\n";
        }

        http::request<http::string_body> forward_req = req;
        forward_req.set(http::field::host, "www.example.com");

        net::io_context ioc;
        net::ip::tcp::resolver resolver{ioc};
        auto const results = resolver.resolve("www.example.com", "80");

        beast::tcp_stream stream{ioc};
        stream.connect(results);

        http::write(stream, forward_req);

        beast::flat_buffer response_buffer;
        http::response<http::string_body> res;
        http::read(stream, response_buffer, res);

        std::cout << "=== Resposta Recebida ===\n";
        std::cout << "Status: " << res.result_int() << "\n";
        std::cout << "Headers:\n";
        for (const auto &header : res)
        {
            std::cout << "  " << header.name_string() << ": " << header.value() << "\n";
        }
        std::cout << "Payload da Resposta: " << res.body() << "\n";

        http::write(socket, res);
    }
    catch (const std::exception &e)
    {
        std::cerr << "Erro ao lidar com requisição: " << e.what() << std::endl;
    }
}