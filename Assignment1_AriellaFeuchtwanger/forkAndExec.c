#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/wait.h>

#define BUFFERSIZE 128

static char *args[BUFFERSIZE], *path_list[BUFFERSIZE];


//Reset the array to all nulls
void clear_array(char **array)
{
	int i;

	// empty all elements of array, set it to null and then set it free
	for(i = 0 ; array[i]; i++) {
		memset(array[i], 0, strlen(array[i]));
		array[i] = NULL;
		free(array[i]);
	}
}

//Get a list of all paths - so you can find all the commands.
void get_path_list(char **envp_helper) {
	int i = 0;
	char *word;
	char *path_string;

	// Use the PATH environment variable to get a list of paths
	path_string = strstr(envp_helper[i], "PATH");
	while(!path_string) {
		i++;
		path_string = strstr(envp_helper[i], "PATH");
	}

	i = 0;
	path_string += 5;


	while ((word = strsep(&path_string, ":"))) {
		path_list[i] = calloc(strlen(word) + 1, sizeof(char*));
		strncat(path_list[i], word, strlen(word));
		strncat(path_list[i], "/", 1);
		i++;
	}
	free(word);
}

//Using the path list we found in the previous function, find the path
//to the command the user wants to use
void get_command_path(char *cmd)
{
	int i;
	char *path = calloc(BUFFERSIZE, sizeof(char*));
	FILE *file;

	// Check the paths
	for(i = 0; path_list[i]; i++) {
		strcpy(path, path_list[i]);
		strncat(path, cmd, strlen(cmd));

		// Figure out what path is the correct one
		if((file = fopen(path, "r"))) {
			strncpy(cmd, path, strlen(path));
			fclose(file);
		}
	}
	free(path);
}

//Change directory command - when forking a process, the original
//parent process will not register the change in directory.

void change_dir(char *path)
{
	char last_backslash[BUFSIZ];
	char path_fix[BUFSIZ];
	char curr_dir[BUFSIZ];
	char *cwdptr = calloc(BUFSIZ, sizeof(char*));

	if(strncmp(path, "/", 1) != 0 && strncmp(path, "..", 2) != 0) {
		getcwd(cwdptr, BUFSIZ);
		strncat(cwdptr, "/", 1);
		strncat(cwdptr, path, strlen(path));
		strncat(cwdptr, "\0", strlen(path));
		strncpy(curr_dir,cwdptr, BUFSIZ);
		if(chdir(curr_dir) == -1)
			perror("chdir");
	// Check if we're going to a parent directory
	} else if (strncmp(path, "..", 2) == 0) {
		getcwd(curr_dir, sizeof(curr_dir));
		strncpy(last_backslash, strrchr(curr_dir, '/'), BUFFERSIZE);
		curr_dir[strlen(curr_dir)-strlen(last_backslash)] = '\0';
		if(chdir(curr_dir) == -1)
			perror("chdir");
	} else {
		if(chdir(path) == -1) {
			getcwd(cwdptr, BUFSIZ);
			strncat(cwdptr, path, strlen(path));
			strncat(cwdptr, "\0", strlen(path));
			strncpy(curr_dir,cwdptr, BUFSIZ);
			if(chdir(curr_dir) == -1)
				perror("chdir");
		}
	}
}

//This is the main part of the assignment - here is where
//the child process is created and run
void execute(char *cmd, char **envp)
{
	// Create the new child process
	if(!fork()) {
		//Execute the command given
		if(execve(cmd, args, envp) == -1) {
			printf("-shell: %s: command not found\n", cmd);
			exit(0);
		}
	} else {
		// If we get here, it's a parent process. Wait until
		//the child finishes running
		wait(NULL);
	}
}

// Get a list of arguments and return the first argument (which is 
// the command)
char *get_args(char *input)
{
	int i = 0;
	char *word, *cmd;

	// Parse the input string into an array of args
	while ((word = strsep(&input," "))) {
		args[i] = calloc(strlen(word) + 1, sizeof(char*));
		strncat(args[i], word, strlen(word));
		i++;
	}

	// Return the first arg - which is the command
	cmd = strncat(args[0], "\0", 1);

	return cmd;
}

// Remove substring
// used in the create_prompt method to remove local from the 
// hostname
void rm_substr(char *str, const char *substr)
{
  while((str = strstr(str, substr)))
    memmove(str, strlen(substr) + str, strlen(strlen(substr) + str) + 1);
}

// Create the prompt
void create_prompt(void)
{
	char hostname[BUFFERSIZE];
	char *path = calloc(BUFSIZ, sizeof(char*));
	char *username = calloc(BUFFERSIZE, sizeof(char*));

	gethostname(hostname, sizeof(hostname));
	rm_substr(hostname, ".local");

	getcwd(path, BUFSIZ);
	path = strrchr(path, '/') + 1;

	username = getlogin();

	printf("%s:%s %s$ ", hostname, path, username);
}

// The driver of the shell program
int main(int argc, char **argv, char **envp) 
{
	char last;
	char *input_str = calloc(BUFFERSIZE, sizeof(char*));
	char *cmd = calloc(BUFFERSIZE, sizeof(char*));

	// Initialize - get the path and create the prompt for the first
	// time.
	create_prompt();
	fflush(stdout);
	get_path_list(envp);
	
	while(strcmp(input_str, "quit") != 0) {
		// get input
		last = getchar();
		// this switch-case statement read and prepare the input string
		switch(last) {
			case '\n':
				// if no command is inserted, just print the prompt again
				if(input_str[0] != '\0') {
					// erase cmd variable
					memset(cmd, 0, BUFFERSIZE);
					// parse the command line
					cmd = get_args(input_str);
					// special case: change directory call
					if(strncmp(cmd, "cd", 2) == 0) {
						change_dir(args[1]);
					// all other command calls
					} else {
						get_command_path(cmd);
						execute(cmd, envp);
					}
					clear_array(args);
				}
				// print the prompt again after hitting enter
				create_prompt();
				memset(input_str, 0, BUFFERSIZE);
				break;
			default:
				// prepare the input string
				strncat(input_str, &last, 1);
				break;
		}
	}

	// free allocated memories
	free(cmd);
	free(input_str);
	clear_array(path_list);

	// print new line if 'ctrl+d' is pressed
	if(last == EOF) printf("\n");

	// end of main
	return 0;
}