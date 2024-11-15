#include <string>
#include <vector>


class Server {

public:
    Server(int port);
    ~Server();	

    int accept_connection();

    int handle_get(std::string filename);
    int handle_put(std::string filename);
    int handle_ls(std::string filename);

private:
    std::vector<int> client_fds;
    
};
