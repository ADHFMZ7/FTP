#include <string>
#include <arpa/inet.h>

class Client {

public: 

    Client(const std::string &host, int port);
    ~Client();
    bool establish_connection();

    int put_file(std::string filename);
    int get_file(std::string filename);
    int get_file_list();

private:

    int sock;
    struct sockaddr_in server_address;
    
    const std::string host;
    const int port;
};
