#include <iostream>
#include <sstream>
#include <string>
#include "client.h"
#include <netdb.h>

#include <readline/readline.h>
#include <readline/history.h>

// Handles an ftp command
// Supported:
// - ls
// - get [filename]
// - put [filename]
// - quit

int handle_command(Client &c, std::string command) {

    std::string op;
    std::string p1;
    std::istringstream stream(command);

    stream >> op;
    getline(stream, p1);
    p1.erase(0, 1);

    if (op == "quit") {
        std::cout << "Exiting ftp client..." << std::endl;
        exit(0);
    }
    else if (op == "ls") {
        c.get_file_list();
    }

    // make sure p1 is a real filename?

    else if (op == "get") {
        std::cout << "getting filename: " << p1 << std::endl;
        c.get_file(p1);

    }
    else if (op == "put") {
        c.put_file(p1);
    }
    else {
        std::cout << "Command not found: " << op << std::endl;
    }
    return 0;
}
int main(int argc, char *argv[]) {

    if (argc != 3) {
        std::cerr << "Usage: " << argv[0] << " <host> <port>" << std::endl;
        return 1;
    }

    std::string host = argv[1];
    int port = std::stoi(argv[2]);

    struct addrinfo *sa;

    if (getaddrinfo(host.c_str(), NULL, NULL, &sa) != 0) {
        std::cerr << "Invalid host: " << host << std::endl;
        return 1;
    }

    struct sockaddr* addrsPtr = sa->ai_addr;

    std::cout << "IP: " << inet_ntoa((((struct sockaddr_in *)addrsPtr)->sin_addr)) << std::endl;

    std::string  ip = inet_ntoa((((struct sockaddr_in *)addrsPtr)->sin_addr));
    freeaddrinfo(sa); 

    Client c(ip, port);

    std::string input;

    while (1) {

        // repl uses readline to provide filename completion and command history
        input = readline("ftp> ");
        add_history(input.c_str());
        handle_command(c, input);

    }
}

