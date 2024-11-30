#include <iostream>
#include <sstream>
#include <string>
#include "client.h"

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
    stream >> p1;

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
int main() {

    Client c("127.0.0.1", 8080);

    std::string input;

    while (1) {

        // repl uses readline to provide filename completion and command history
        input = readline("ftp> ");
        add_history(input.c_str());
        handle_command(c, input);

    }
}

