#include <iostream>
#include <sstream>
#include <string>

#include <readline/readline.h>
#include <readline/history.h>

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


    std::string input;


    while (1) {

        // repl uses readline to provide extra functionality like file completion and command history
        input = readline("ftp> ");
        add_history(input.c_str());
        handle_command(input);

    }



}

