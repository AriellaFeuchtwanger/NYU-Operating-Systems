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

	//num_char += i;
	return i;
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

char **messagepass_1_svc(char **a, struct svc_req *req){
	//Preliminary preparation:
	int count;
	static char msg[256];
	static char *string;
	char line[BUFSIZ];
	FILE * secrets;

	//Set up CTRL-C interrupt handler
	//Ignore CTRL-C
	signal(SIGINT, SIG_IGN);
	//Assign CTRL-C to the info function
	signal(SIGINT, show_server_info);

	//Increase number of lines sent
	num_line++;
	//Check if we want to quit - if we do, there's no need to write
	//the line to the file. Also, we then need to exit the program,
	//because otherwise the server will keep running forever (or 
	//until manually killed)
	if ((strncmp(*a, "quit\n", 5) == 0) || (strncmp(*a, "QUIT\n", 5) == 0)){
			exit(0);
	} 

	//Open the file - this will create the file if it does
	//not exist yet.
	secrets = fopen("secrets.out", "a");

	if(secrets != NULL){
		fprintf(secrets, "%d: ", num_characters(*a));
		fputs(*a, secrets);
		fclose(secrets);
	}

	string = "Sent!";

	return(&string);

}