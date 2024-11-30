#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <sys/wait.h>
#include <signal.h>
#include <string>
#include <filesystem>
#include <netinet/tcp.h>
#include <fstream>
#include <iostream>

#define BACKLOG 10	 // how many pending connections queue will hold

int send_error_header(int sockfd) {
	uint32_t header = 0;
	if (send(sockfd, &header, 4, 0) == -1) {
		perror("send");
		return -1;
	}
	return 0;
}

void sigchld_handler(int s)
{
	(void)s; // quiet unused variable warning

	// waitpid() might overwrite errno, so we save and restore it:
	int saved_errno = errno;

	while(waitpid(-1, NULL, WNOHANG) > 0);

	errno = saved_errno;
}

// get sockaddr, IPv4 or IPv6:
void *get_in_addr(struct sockaddr *sa)
{
	if (sa->sa_family == AF_INET) {
		return &(((struct sockaddr_in*)sa)->sin_addr);
	}

	return &(((struct sockaddr_in6*)sa)->sin6_addr);
}

int main(int argc, char *argv[])
{
	int sockfd, new_fd;  // listen on sock_fd, new connection on new_fd
	struct addrinfo hints, *servinfo, *p;
	struct sockaddr_storage their_addr; // connector's address information
	socklen_t sin_size;
	struct sigaction sa;
	int yes=1;
	char s[INET6_ADDRSTRLEN];
	int rv;

	if (argc != 2) {
		fprintf(stderr,"usage: %s <port>\n", argv[0]);
		exit(1);
	}

	// Set the port number
	char *port = argv[1];

	// Set the port number	

	memset(&hints, 0, sizeof hints);
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_flags = AI_PASSIVE; // use my IP

	if ((rv = getaddrinfo(NULL, port, &hints, &servinfo)) != 0) {
		fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
		return 1;
	}

	// loop through all the results and bind to the first we can
	for(p = servinfo; p != NULL; p = p->ai_next) {
		if ((sockfd = socket(p->ai_family, p->ai_socktype,
				p->ai_protocol)) == -1) {
			perror("server: socket");
			continue;
		}

		if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &yes,
				sizeof(int)) == -1) {
			perror("setsockopt");
			exit(1);
		}

		if (bind(sockfd, p->ai_addr, p->ai_addrlen) == -1) {
			close(sockfd);
			perror("server: bind");
			continue;
		}

		break;
	}

	freeaddrinfo(servinfo); // all done with this structure


	if (p == NULL)  {
		fprintf(stderr, "server: failed to bind\n");
		exit(1);
	}

	if (listen(sockfd, BACKLOG) == -1) {
		perror("listen");
		exit(1);
	}

	sa.sa_handler = sigchld_handler; // reap all dead processes
	sigemptyset(&sa.sa_mask);
	sa.sa_flags = SA_RESTART;
	if (sigaction(SIGCHLD, &sa, NULL) == -1) {
		perror("sigaction");
		exit(1);
	}

	std::cout << "Server started on port " << port << std::endl;

	printf("server: waiting for connections...\n");

	while(1) {  // main accept() loop
		sin_size = sizeof their_addr;
		new_fd = accept(sockfd, (struct sockaddr *)&their_addr, &sin_size);
		if (new_fd == -1) {
			perror("accept");
			continue;
		}

		inet_ntop(their_addr.ss_family,
			get_in_addr((struct sockaddr *)&their_addr),
			s, sizeof s);
		printf("server: got connection from %s\n", s);
		int mss;
		socklen_t optlen = sizeof(mss);

		if (getsockopt(new_fd, IPPROTO_TCP, TCP_MAXSEG, &mss, &optlen) == -1) {
			perror("getsockopt failed");
			close(sockfd);
			exit(EXIT_FAILURE);
		}


		if (!fork()) { // this is the child process
			close(sockfd); // child doesn't need the listener

			char buf[mss];

			int bytes_received = 0;

			// Loop to handle multiple client commands
			while (1) {
				bytes_received = recv(new_fd, buf, mss - 1, 0);
				if (bytes_received <= 0) break; 

				buf[bytes_received] = '\0'; 
				std::string command (buf);
				if (command == "ls") {
					
					// TODO: Handle the case where the ls output is longer than MSS

					std::string file_list;
					std::string path = "./";
					for (const auto &entry : std::filesystem::directory_iterator(path)) {
						file_list.append(entry.path().filename().string());
						file_list.append("\n");
					}

					if (send(new_fd, file_list.c_str(), file_list.length(), 0) == -1)
						perror("send");
				}
				else if (command.substr(0, command.find(" ")) == "get") {

					std::string filename = command.substr(command.find(" ") + 1);
					filename.erase(filename.find_last_not_of(" \n\r\t") + 1);  // Trim trailing whitespace

					// ensure file exists
					if (!std::filesystem::exists(filename)) {
						std::cerr << "File not found: " << filename << std::endl;

						send_error_header(new_fd);	
						continue;
					}

					FILE *file = fopen(filename.c_str(), "rb");
					if (!file) {
						send_error_header(new_fd);
						perror("Failed to open file");
						return -1;
					}	

					std::cout << "Sending file: " << filename << std::endl;

					int bytes_read;
					int chunk_size = mss - 4;
					uint16_t seq_num = 0;

					while ((bytes_read = fread(buf + 4, 1, chunk_size, file)) > 0) {
						buf[0] = (bytes_read >> 8) & 0xFF;
						buf[1] = bytes_read & 0xFF;
						buf[2] = (seq_num >> 8) & 0xFF;
						buf[3] = seq_num & 0xFF;

						std::cout << "Read " << bytes_read << " bytes; seq number: " << seq_num << std::endl;

						if (send(new_fd, buf, bytes_read + 4, 0) == -1) {
							perror("Failed to send segment");
							fclose(file);
							return -1;
						}

						seq_num++; 
					}	

					if (bytes_read == 0) {
						// EOF marker: Length = 0, Sequence number = last seq_num
						buf[0] = 0x00; // Length high byte
						buf[1] = 0x00; // Length low byte
						buf[2] = ((seq_num) >> 8) & 0xFF; // Seq number high byte
						buf[3] = (seq_num) & 0xFF;       // Seq number low byte

						if (send(new_fd, buf, 4, 0) == -1) {
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
				}

				else if (command.substr(0, command.find(" ")) == "put") {
					std::string filename = command.substr(command.find(" ") + 1);
					filename.erase(filename.find_last_not_of(" \n\r\t") + 1);  // Trim trailing whitespace
					std::ofstream ofs(filename, std::ofstream::out | std::ofstream::binary);

					if (!ofs.is_open()) {
						std::cerr << "Failed to open file: " << filename << std::endl;
						continue;
					}

					std::cout << "Receiving file: " << filename << std::endl;

					while (1) {
						// Read the 4-byte header
						bytes_received = recv(new_fd, buf, 4, 0);
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


						if (length == 0) {
							std::cout << "EOF marker received" << std::endl;
							break;
						}

						// Read the data payload
						bytes_received = recv(new_fd, buf, length, 0);
						if (bytes_received <= 0) {
							perror("recv error or connection closed unexpectedly");
							break;
						}

						if (bytes_received != length) {
							std::cerr << "Incomplete data received. Expected: " << length
									<< ", Received: " << bytes_received << std::endl;
							break;
						}

						ofs.write(buf, bytes_received);
						if (ofs.bad()) {
							std::cerr << "Error writing to file: " << filename << std::endl;
							break;
						}
					}

					ofs.close();
					std::cout << "File transfer complete: " << filename << std::endl;
				}

				else {
					// unrecognized command error
					std::cout << "Command not found: " << command.substr(0, command.find(" ")) << std::endl;
				}
			}

			close(new_fd); 
			exit(0);
		}
		close(new_fd);  
	}

	return 0;
}
