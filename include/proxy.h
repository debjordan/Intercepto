#ifndef PROXY_H
#define PROXY_H

#include <boost/asio.hpp>
#include <boost/beast.hpp>
#include <string>

namespace intercepto {

class Proxy {
public:
    void start(const std::string& host, unsigned short port);

private:
    void handle_request(boost::asio::ip::tcp::socket& socket);
    void forward_request(boost::asio::ip::tcp::socket& socket, boost::beast::http::request<boost::beast::http::string_body>& req);
    void send_response(boost::asio::ip::tcp::socket& socket, boost::beast::http::response<boost::beast::http::string_body>& res);
};

}

#endif // PROXY_H
