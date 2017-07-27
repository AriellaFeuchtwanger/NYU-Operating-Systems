#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>

#define SHAREDMEMSIZE 1024

int main(void){
	//Preliminary Preparartion

	//Variables:
	key_t key;
	int shared_mem_id;
	int run = 1; //run the loop - initialized to true
	char *shared_mem, *string;
	char *input = malloc(sizeof(char)*BUFSIZ);

	key = getuid(); //key of the shared mem is the user id

	shared_mem_id = shmget(key, SHAREDMEMSIZE, IPC_CREAT | 0666); //create shared mem

	// if(shared_mem_id < 0){
	// 	pererror("ERROR: Shared Memory not created");
	// 	exit(EXIT_FAILURE);
	//}

	shared_mem = shmat(shared_mem_id, NULL, 0); //attach shared_mem to dataspace

	// if(shared_mem == (char *) -1){//ensure shared_mem attached
	// 	pererror("ERROR: Shared Memory not attached");
	// 	exit(EXIT_FAILURE);
	// }

	//Begin loop

	while(run){
		printf("Enter your secret code or QUIT to exit: "); //print prompt for information
		fgets(input, BUFSIZ, stdin); //get user input, put into input

		//Add a quit possibility - mostly for my sanity
		if (strncmp(input, "quit", 4) == 0){
			memcpy(shared_mem, input, strlen(input)); //copy input to shared memory

			string = shared_mem;
			string += strlen(input);
			*string = ';';
			run = 0; //set run to false
			while(*shared_mem != '>')//processor uses > to show it has read the line
				sleep(1);
		} else if (strncmp(input, "C00L", 4) == 0){ //check that it's the correct line

			memcpy(shared_mem, input, strlen(input)); //copy input to shared memory

			string = shared_mem;
			string += strlen(input);
			*string = ';'; //use semicolon to show end of line
			while(*shared_mem != '>')//processor uses > to show it has read the line
				sleep(1);
		}

		
	}

	//Release shared memory and IPC communication
	shmdt(shared_mem);
	shmctl(shared_mem_id, IPC_RMID, 0);
}