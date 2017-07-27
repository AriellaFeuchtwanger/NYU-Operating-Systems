#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <netdb.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <arpa/inet.h>

#define _BSD_SOURCE
#define PORT "5000"

//get the IP address
void get_ip_address(char *ip_address)
{

	char buffer[200];
	struct hostent* host;

	gethostname(buffer, 200);
	host = (struct hostent *) gethostbyname(buffer);

	strcpy(ip_address, inet_ntoa(*((struct in_addr *)host->h_addr)));
}

//get socket address
void *get_socket_address(struct sockaddr *socket_address)
{
	//check if ipv4
	if(socket_address->sa_family == AF_INET){
		return &(((struct sockaddr_in*)socket_address)->sin_addr);
	}

	return &(((struct sockaddr_in6*)socket_address)->sin6_addr);
}

//Send the data over to the server
void send_data(char *input){
	//Variables
	int sockfd, result = 1;
	struct addrinfo server_address, *server_info, *p;
	char ip_address[200], s[INET6_ADDRSTRLEN];
	//Server address:
		memset(&server_address, 0, sizeof server_address);
		server_address.ai_family = AF_UNSPEC;
		server_address.ai_socktype = SOCK_STREAM;

		//Get IP address
		get_ip_address(ip_address);

		//Address info:
		if((result = getaddrinfo(ip_address, PORT, &server_address, &server_info)) != 0)
		{
			//Error
			fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(result));
			//return 1;
		}

		//Go through server_info and bind it to the first option available
		for(p = server_info; p != NULL; p = p->ai_next){
			if((sockfd = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) < 0){
				perror("Error in client: socket was not created");
				continue;
			}

			if(connect(sockfd, p->ai_addr, p->ai_addrlen) < 0){
				close(sockfd);
				perror("Error in client: socket did not connect");
				continue;
			}

			break;
		}

		//Make sure the client connected:
		if(p == NULL){
			fprintf(stderr, "Error in client: socket did not connect\n");
			//return 2;
		}

		//Convert addresses from binary to text:
		inet_ntop(p->ai_family, get_socket_address((struct sockaddr *)p->ai_addr), s, sizeof s);

		//Print out IP address of server
		printf("Client: connecting to %s\n", s);

		//Remove the server_info - no need for it anymore
		freeaddrinfo(server_info);

		if(send(sockfd, input, strlen(input), 0) < 0){
			perror("Send error");
		}
		close(sockfd);
}

int main(void){
	char *input = malloc(sizeof(char)*BUFSIZ);
	int run = 1;

	
	while(run){

		printf("Enter your secret code or QUIT to exit: "); //print prompt for information
		fgets(input, BUFSIZ, stdin); //get user input, put into input

		//Add a quit possibility - mostly for my sanity
		if (strncmp(input, "quit\n", 5) == 0 || strncmp(input, "QUIT\n", 5) == 0){
			send_data(input);
			run = 0; //set run to false
		} else if (strncmp(input, "C00L\n", 5) == 0){ //check that it's the correct line
			send_data(input);
		}
		
	}
	return 0;
}