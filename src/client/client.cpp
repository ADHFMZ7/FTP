#include "client.h"
#include <cstdlib>
#include <iostream>
#include <fstream>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/tcp.h>

#define BUFLEN 2048

Client::Client(const std::string &host, int port): host(host), port(port) {

    if (!establish_connection()) {
        std::cerr << "Failed to establish connection" << std::endl;
        exit(1);
    }
}

Client::~Client() {
    close(this->sock);
    delete[] buf;
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
    
    buf = new char[mss];
    return true;
}

int Client::put_file(std::string filename) {

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


    uint16_t seq_num = 0; 
    size_t bytes_read;

    while ((bytes_read = fread(buf + 4, 1, chunk_size, file)) > 0) {
        buf[0] = (bytes_read >> 8) & 0xFF;
        buf[1] = bytes_read & 0xFF;
        buf[2] = (seq_num >> 8) & 0xFF;
        buf[3] = seq_num & 0xFF;

        std::cout << "Read " << bytes_read << " bytes; seq number: " << seq_num << std::endl;

        if (send(sock, buf, bytes_read + 4, 0) == -1) {
            perror("Failed to send segment");
            fclose(file);
            return -1;
        }

        seq_num++; 
    }

    if (bytes_read == 0) {
        // EOF marker: Length = 0, Sequence number = last seq_num + 1
        buf[0] = 0x00; // Length high byte
        buf[1] = 0x00; // Length low byte
        buf[2] = ((seq_num + 1) >> 8) & 0xFF; // Seq number high byte
        buf[3] = (seq_num + 1) & 0xFF;       // Seq number low byte

        if (send(sock, buf, 4, 0) == -1) {
            perror("Failed to send EOF marker");
            fclose(file);
            return -1;
        }

        std::cout << "Sent EOF marker" << std::endl;
    } 


    if (ferror(file)) {
        perror("File read error");
    } else {
        std::cout << "File transfer complete: " << filename << std::endl;
    }

    fclose(file);

    return 0;
}

int Client::get_file(std::string filename) {
    // receive file from server

    // send get request
    std::string request = "get " + filename;
    if (send(sock, request.c_str(), request.length(), 0) == -1) {
        perror("Failed to send get request");
        return -1;
    }

    std::ofstream ofs(filename, std::ofstream::out | std::ofstream::binary);

    int bytes_received;

    while (1) {
        // Read the 4-byte header
        bytes_received = recv(sock, buf, 4, 0);
        if (bytes_received == 0) {
            perror("recv error");
            break;
        }

        if (bytes_received != 4) {
            std::cerr << "Incomplete header received. Expected 4 bytes, got " 
                    << bytes_received << " bytes." << std::endl;
            break;
        }

        // Parse the header
        uint16_t length = (static_cast<unsigned char>(buf[0]) << 8) |
                        static_cast<unsigned char>(buf[1]);
        uint16_t seq_num = (static_cast<unsigned char>(buf[2]) << 8) |
                        static_cast<unsigned char>(buf[3]);

        std::cout << "Segment received - Length: " << length
                << ", Sequence Number: " << seq_num << std::endl;

        // Validate length
        if (length > mss - 4) {
            std::cerr << "Invalid length: " << length << " exceeds MSS payload." << std::endl;
            break;
        }

        if (length == 0 && seq_num == 0) {
            std::cout << "File cannot be sent" << std::endl;
            ofs.close();
            remove(filename.c_str());
            break;
        }

        if (length == 0) {
            std::cout << "EOF marker received" << std::endl;
            break;
        }

        // Read the data payload
        bytes_received = recv(sock, buf, length, 0);
        if (bytes_received <= 0) {
            perror("recv error or connection closed unexpectedly");
            ofs.close();
            remove(filename.c_str());
            break;
        }

        if (bytes_received != length) {
            std::cerr << "Incomplete data received. Expected: " << length
                    << ", Received: " << bytes_received << std::endl;
            break;
        }

        // write to file
        ofs.write(buf, bytes_received);
        if (ofs.bad()) {
            std::cerr << "Error writing to file: " << filename << std::endl;
            ofs.close();
            remove(filename.c_str());
            break;
        }
    }

    ofs.close();
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
