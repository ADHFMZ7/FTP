#include "client.h"
#include <cstdlib>
#include <iostream>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/tcp.h>

#define BUFLEN 2048

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
        exit(1);
    }
    std::cout << "Connected to server" << std::endl;

    socklen_t optlen = sizeof(mss);

    if (getsockopt(sock, IPPROTO_TCP, TCP_MAXSEG, &mss, &optlen) == -1) {
        perror("getsockopt failed");
        close(sock);
        exit(1);
    }


    return true;
}

int Client::put_file(std::string filename) {

    // request
    std::string request = "put " + filename;
    if (send(sock, request.c_str(), request.length(), 0) == -1) {
        perror("Failed to send put request");
        return -1;
    }

    FILE *file = fopen(filename.c_str(), "rb");
    if (!file) {
        perror("Failed to open file");
        return -1;
    }

    int chunk_size = mss - 4;
    std::cout << "Using chunk size: " << chunk_size << " bytes" << std::endl;

    char *buffer = new char[mss];

    uint16_t seq_num = 0; 
    size_t bytes_read;

    while ((bytes_read = fread(buffer + 4, 1, chunk_size, file)) > 0) {
        buffer[0] = (bytes_read >> 8) & 0xFF;
        buffer[1] = bytes_read & 0xFF;
        buffer[2] = (seq_num >> 8) & 0xFF;
        buffer[3] = seq_num & 0xFF;

        if (send(sock, buffer, bytes_read + 4, 0) == -1) {
            perror("Failed to send segment");
            delete[] buffer;
            fclose(file);
            return -1;
        }

        seq_num++; 
    }

    if (ferror(file)) {
        perror("File read error");
    } else {
        std::cout << "File transfer complete: " << filename << std::endl;
    }

    delete[] buffer;
    fclose(file);

    return 0;
}


int Client::get_file(std::string filename) {

    std::string request = "get " + filename;
    send(sock, request.c_str(), request.length(), 0);

    return 0;
}

int Client::get_file_list() {

    send(sock, "ls", 3, 0);

    char *buf = (char *) malloc(sizeof(char) * BUFLEN);

    int bytes_read = recv(this->sock, buf, BUFLEN, 0);
    if (bytes_read == -1) {
        std::cerr << "Failed to receive message" << std::endl;
        return false;
    }
    std::cout << buf; 

    free(buf);
    return 0;
}
