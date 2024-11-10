#include "client.h"
#include <iostream>
#include <unistd.h>
#include <sys/socket.h>

Client::Client(const std::string &host, int port): host(host), port(port) {

    if (!establish_connection()) {
        // handle error  
    }

}

Client::~Client() {
    close(this->sock);
    std::cout << "Connection Closed" << std::endl;
}

bool Client::establish_connection() {
    sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock < 0) {
        std::cerr << "Socket creation error" << std::endl;
        return false;
    }

    server_address.sin_family = AF_INET;
    server_address.sin_port = htons(port);

    if (inet_pton(AF_INET, host.c_str(), &server_address.sin_addr) <= 0) {
        std::cerr << "Invalid address or Address not supported" << std::endl;
        return false;
    }

    if (connect(sock, (struct sockaddr*)&server_address, sizeof(server_address)) < 0) {
        std::cerr << "Connection failed" << std::endl;
        return false;
    }
    std::cout << "Connected to server" << std::endl;
    return true;
}

int Client::put_file(std::string filename) {
    
    return 0;
}

int Client::get_file(std::string filename) {
    
    return 0;
}

int Client::get_file_list() {

    return 0;
}
