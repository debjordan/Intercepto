#ifndef PROXY_H
#define PROXY_H

#include <boost/asio.hpp>
#include <boost/beast.hpp>
#include <string>

class Proxy
{
public:
    void start(const std::string &host, unsigned short port);

private:
    void handle_request(boost::asio::ip::tcp::socket socket);
};

#endif