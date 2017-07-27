#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <utmp.h> 
#include <rpc/rpc.h>
#include <rpcsvc/rusers.h>
#include "messagepass.h"

int main(int argc, char *argv[]){
	CLIENT *cl;
	int run = 1; //run the loop - initialized to true
	char *string;
	char *input = malloc(sizeof(char)*BUFSIZ);
	char **msg;

	//Check and make sure that there's an IP address
	if(argc != 2){
		printf("Usage: client hostname \n");
		exit(1);
	}

	//Set up RPC Client:
	cl = clnt_create(argv[1], MESSAGE_PROG, MESSAGE_VERS, "tcp");
	if(cl == NULL){
		clnt_pcreateerror(argv[1]);
		exit(1);
	}

	//Begin loop
	while(run){
		printf("Enter your secret code or QUIT to exit: "); //print prompt for information
		fgets(input, BUFSIZ, stdin); //get user input, put into input

		//Add a quit possibility - mostly for my sanity
		if (strncmp(input, "quit", 4) == 0){
			run = 0; //set run to false
		} 
		printf("Sending along the message to the server...\n");
		//Using the messagepass_1 function created by rpcgen, send
		//the line to the server to be put in secrets.out
		msg = messagepass_1(&input, cl);

		printf("We're back!\n");

		//Check if the message was sent
		if(msg == NULL){
			//First, make sure that the server didn't end,
			//which it will do if the user typed in 'quit'
			if ((strncmp(input, "quit\n", 5) == 0) || (strncmp(input, "QUIT\n", 5) == 0)){
				printf("Exiting...\n");
				exit(0);
			} else{ 
				//Here, tell the user that there is an actual
				//error to be aware of
				clnt_perror(cl, argv[1]);
				exit(1);
			}
		} else{
			//If everything worked properly,
			//the result should be 'Sent!'
			printf("Result: %s\n", *msg);
		}
		
	}
	return 0;
}