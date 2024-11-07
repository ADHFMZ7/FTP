#include <iostream>
#include <sstream>
#include <string>

// Handles an ftp command
// Supported:
// - ls
// - get [filename]
// - put [filename]
// - quit
int handle_command(std::string command) {

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
        std::cout << "ls..." << std::endl;
        
    }
    else if (op == "get") {
        std::cout << "getting filename: " << p1 << std::endl;

    }
    else if (op == "put") {
        std::cout << "putting filename: " << p1 << std::endl;

    }

}
int main() {


    bool active = true;
    std::string input;

    // Here just a repl

    while (active) {
        std::cout << "ftp> ";
        std::getline(std::cin, input);

        handle_command(input);

    }



}

