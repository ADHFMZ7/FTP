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

#define PORT "8080"  // the port users will be connecting to
#define BACKLOG 10	 // how many pending connections queue will hold

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

int main(void)
{
	int sockfd, new_fd;  // listen on sock_fd, new connection on new_fd
	struct addrinfo hints, *servinfo, *p;
	struct sockaddr_storage their_addr; // connector's address information
	socklen_t sin_size;
	struct sigaction sa;
	int yes=1;
	char s[INET6_ADDRSTRLEN];
	int rv;

	memset(&hints, 0, sizeof hints);
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_flags = AI_PASSIVE; // use my IP

	if ((rv = getaddrinfo(NULL, PORT, &hints, &servinfo)) != 0) {
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
			std::string command;

			int bytes_received = 0;

			// Loop to handle multiple client commands
			while (true) {
				bytes_received = recv(new_fd, buf, mss - 1, 0);
				if (bytes_received <= 0) break; // connection closed by client or error

				buf[bytes_received] = '\0'; // null-terminate for safety
				command = buf;
				if (command == "ls") {
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
				
					// ensure file exists

					// calculate number of segments
					
					// send number of segments

					// loop n times sending segments

				}
				else if (command.substr(0, command.find(" ")) == "put") {
					
					bytes_received = recv(new_fd, buf, mss - 1, 0);
					if (bytes_received <= 0) break;

					std::ofstream ofs (command.substr(command.find(" ") + 1), std::ofstream::out);


					// ERROR CHECKING HERE

					ofs << "LOREM IPSUM";
					if(ofs.bad())    //bad() function will check for badbit
					{
						std::cout << "Writing to file failed" << std::endl;
					}
					ofs.close();
					std::cout << "written to file" << std::endl;
					// get number of segments


					// buf should contain a 32 bit int	
					


					// loop n times and recv segments

				}

				else {
					// unrecognized command error
					std::cout << "Command not found: " << command.substr(0, command.find(" ")) << std::endl;
				}
			}

			close(new_fd); // close connection when done
			exit(0);
		}
		close(new_fd);  // parent doesn't need this
	}

	return 0;
}
