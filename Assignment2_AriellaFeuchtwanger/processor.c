#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>

#define SHAREDMEMSIZE 1024

int main(void){
	//Preliminary preparation:
	key_t key;
	int shared_mem_id;
	int count;
	int run = 1; //variable to keep loop running
	char *shared_mem, *string;
	char line[BUFSIZ];
	FILE * secrets;

	secrets = fopen("secrets.out", "w"); //create file

	key = getuid(); //set key to user id - just like it was set up in receiver.c

	shared_mem_id = shmget(key, SHAREDMEMSIZE, IPC_CREAT | 0666); //create shared mem

	// if(shared_mem_id < 0){
	// 	pererror("ERROR: Shared Memory not created");
	// 	exit(EXIT_FAILURE);
	// }

	shared_mem = shmat(shared_mem_id, NULL, 0); //attach shared_mem to dataspace

	// if(shared_mem == (char *) -1){//ensure shared_mem attached
	// 	pererror("ERROR: Shared Memory not attached");
	// 	exit(EXIT_FAILURE);
	// }

	while(run){
		if(*shared_mem != (int) NULL){
			if(*shared_mem != '>'){
				count = 0;
				memset(line, 0, sizeof(line));

				//get string through shared memory
				//semicolon shows end of line
				for(string = shared_mem; *string != ';'; string++){
					line[count] = *string;
					count++;
				}

				//check if user is trying to quit
				if(strncmp(line, "quit", 4) == 0){
					run = 0;
				}

				*shared_mem = '>'; //signal ready for more

				//Write to the 'secrets.out' file:
				secrets = fopen("secrets.out", "a");

				if(secrets != NULL){
					fprintf(secrets, "%d: ", count - 1);
					fputs(line, secrets);
					fclose(secrets);
				}
			}
		}
	}
	shmdt(shared_mem); //release shared memory
	exit(0);
}