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
#include <ctype.h>

#define PORT "5000"
#define BACKLOG 10

int num_char = 0;
int num_line = 0;

//Count the number of digits in a line - used to keep track
//of the number of digits in the global variable num_char
//as well as in the file
int num_characters(char *s)
{
	int i = 0;
	while(*s != '\0'){
		if(isdigit(*s)) //Check that this is a digit (since the secret
			//code is C00L, each line has 2 characters and 2 digits)
			i++;
		s++;
	}

	num_char += i;
	return i;
}

//To release all the dea processes
static void sigchild_handler(int sig)
{
	//Wait for all dead processes to finish
	while(waitpid(-1, NULL, WNOHANG) > 0){

	}
}

//Get the socket address:
void *get_socket_address(struct sockaddr *sock_address){
	if(sock_address->sa_family == AF_INET){
		return &(((struct sockaddr_in*)sock_address)->sin_addr);
	}

	return &(((struct sockaddr_in6*)sock_address)->sin6_addr);
}

//For the interrupt handler - when CTRL-C is clicked,
//this will display the number of lines received as well as the 
//number of digits
void show_server_info(int signo){
	if (signo != 0)
		printf("\n");
	printf("Number of lines: %d\n", num_line);
	printf("Number of digits: %d\n", num_char);
}

//Meat of the program
int main(void){
	//Preliminary preparation:
	int sock_fd, new_fd, result, buf_len, sso, run = 1;
	struct addrinfo server_address, *server_info, *p;
	struct sockaddr_storage client_address;
	socklen_t sin_size;
	struct sigaction sa;
	char s[INET6_ADDRSTRLEN], line[BUFSIZ];
	FILE * secrets;

	//Set up CTRL-C interrupt handler
	//Ignore CTRL-C
	signal(SIGINT, SIG_IGN);
	//Assign CTRL-C to the info function
	signal(SIGINT, show_server_info);

	secrets = fopen("secrets.out", "w"); //create file
	fclose(secrets);

	//Set up the server information:
	memset(&server_address, 0, sizeof server_address);
	server_address.ai_family = AF_UNSPEC;
	server_address.ai_socktype = SOCK_STREAM;
	server_address.ai_flags = AI_PASSIVE;

	//Get address info. Error if fails.
	if((result = getaddrinfo(NULL, PORT, &server_address, &server_info))!= 0){
		fprintf(stderr, "getaddrinfo:%s\n", gai_strerror(result));
		return 1;
	}

	//Go through server_info:
	for(p = server_info; p != NULL; p = p->ai_next){
		if((sock_fd = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) < 0){
			perror("Error in server: socket was not created");
			continue;
		}

		if(setsockopt(sock_fd, SOL_SOCKET, SO_REUSEADDR, &sso, sizeof(int)) < 0){
			perror("setsockopt: failed. Exiting...");
			exit(1);
		}

		if(bind(sock_fd, p->ai_addr, p->ai_addrlen) < 0){
			close(sock_fd);
			perror("Error in server: socket did not bind");
			continue;
		}
		break;

	}

	//Make sure server binded successfully:
	if(p == NULL){
		fprintf(stderr, "Error in Server: failed to bind. Exiting...");
		return 2;
	}

	//Get rid of server_info - not needed
	freeaddrinfo(server_info);

	//Listen to incoming sockets for data
	if(listen(sock_fd, BACKLOG) < 0){
		perror("listen: failed. Exiting...");
		exit(1);
	}

	//Get rid of dead processes
	sa.sa_handler = sigchild_handler;
	sigemptyset(&sa.sa_mask);
	sa.sa_flags = SA_RESTART;

	//Check that it worked properly:
	if(sigaction(SIGCHLD, &sa, NULL) < 0){
		perror("Error in server: sigaction failed. Exiting...");
		exit(1);
	}

	//All ready!
	printf("Server: Ready to connect!\n");

	//Begin main program

	while(run){
		//Accept the incoming socket:
		sin_size = sizeof client_address;
		new_fd = accept(sock_fd, (struct sockaddr *)&client_address, &sin_size);
		if(new_fd < 0){
			perror("Accept!");
			continue;
		}

		//Convert IP address to text form:
		inet_ntop(client_address.ss_family, get_socket_address((struct sockaddr *)&client_address), s, sizeof s);

		printf("Server: Connecting to...%s\n", s);

		//Accept the buffer. Error if there's a problem:
		if((buf_len = recv(new_fd, line, BUFSIZ-1, 0)) < 0){
			perror("recv: Could not receive from client. Exiting...");
			exit(1);
		}

		//Add a null symbol to the end to show the end of the string
		line[buf_len] = '\0';
		//num_characters(line);

		printf("Server: Received %s from client\n", line);

		//Quit sequence - for my sanity
		if(strncmp(line, "quit", 4) == 0 || strncmp(line, "QUIT", 4) == 0){
			run = 0;
		}

		//Check for the secret code - and only write the line if it 
		//is (this deals with a quit problem or a line that is not sent)
		if(strncmp(line, "C00L", 4) == 0){
			num_line++;
			//Open file to write line
			secrets = fopen("secrets.out", "a");

			//Make sure the file opens
			if(secrets != NULL){
				fprintf(secrets, "%d: ", num_characters(line));
				fputs(line, secrets);
				fclose(secrets);
			}
		}

		//Close the socket to make way for more
		close(new_fd);

	}
	return 0;
}