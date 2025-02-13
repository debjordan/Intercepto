#include <iostream>
#include "proxy.h"

int main(int argc, char *argv[])
{
    if (argc != 3)
    {
        std::cerr << "Uso: " << argv[0] << " <host> <porta>" << std::endl;
        return 1;
    }

    std::string host = argv[1];
    unsigned short port = std::stoi(argv[2]);

    Proxy proxy;
    proxy.start(host, port);

    return 0;
}