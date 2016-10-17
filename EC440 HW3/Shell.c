#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <string.h>
#include <fcntl.h>

void parser();
void INThandler(int);
int FORK_EXEC(int argc, char **argv){
	int pid;

	switch (pid = fork()) {
	case -1:  //errors
		perror("fork");
		return 1;
	case 0:   //child
		execvp(argv[0], argv);
		perror(argv[0]);
		printf("*** Could not execute '%s'\n", argv[0]);
		exit(1);  // non-zero child exit
	default:{ //parent
		int currentstate;
		wait(&currentstate);
		return (currentstate == 0) ? 0 : 1;
	}
	}
}


void APPEND_FUNC(char **token, char *file, int function2){
	int in, out;
	char *args[3];
	pid_t  pid;
	int    currentstate;
	args[0] = token[0];
	args[1] = token[1];
	args[2] = NULL;
	if ((pid = fork()) < 0) {     /* fork a child process           */
		printf("ERROR <forking child failed>\n");
		exit(1);
	}
	else if (function2 == 0 && pid == 0){//>>
		in = open(file, O_RDWR | O_APPEND | O_CREAT, S_IRWXO);
		dup2(in, STDOUT_FILENO);
		close(in);
		execvp(args[0], args);
		perror(args[0]);
		printf("*** Could not execute '%s'\n", args[0]);
		exit(1);  
	}
	else if (function2 == 1 && pid == 0){//2>>
		in = open(file, O_RDWR | O_APPEND | O_CREAT, S_IRWXO);
		dup2(in, STDERR_FILENO);
		close(in);
		execvp(args[0], args);
		perror(args[0]);
		printf("Execution impossible for '%s'\n", args[0]);
		exit(1);
	}

}

void REDIRECT_FUNC(char **token, char *file, int function1){
	int in, out;
	char *args[3];
	pid_t  pid;
	int    currentstate;
	args[0] = token[0];
	args[1] = token[1];
	args[2] = NULL;
	if ((pid = fork()) < 0) {
		printf("*** ERROR: forking child process failed\n");
		exit(1);
	}
	else if (function1 == 0 && pid == 0){//<
		in = open(file, O_RDONLY);
		dup2(in, 0);
		close(in);
		execvp(args[0], args);
		perror(args[0]);
		printf("*** Could not execute '%s'\n", args[0]);
		exit(1);  
	}
	else if (function1 == 1 && pid == 0){//>
		out = open(file, O_WRONLY | O_TRUNC | O_CREAT, S_IRUSR | S_IRGRP | S_IWGRP | S_IWUSR);
		dup2(out, 1);
		close(out);
		execvp(args[0], args);
		perror(args[0]);
		printf("*** Could not execute '%s'\n", args[0]);
		exit(1);  
	}
	else if (function1 == 2 && pid == 0){
		out = open(file, O_WRONLY | O_TRUNC | O_CREAT, S_IRUSR | S_IRGRP | S_IWGRP | S_IWUSR);
		dup2(out, STDERR_FILENO);
		close(out);
		execvp(args[0], args);
		perror(args[0]);
		printf("*** Could not execute '%s'\n", args[0]);
		exit(1); 
	}
	else if (function1 == 3 && pid == 0){
		out = open(file, O_WRONLY | O_TRUNC | O_CREAT, S_IRUSR | S_IRGRP | S_IWGRP | S_IWUSR);
		dup2(out, STDERR_FILENO);
		dup2(out, STDOUT_FILENO);
		close(out);
		execvp(args[0], args);
		perror(args[0]);
		printf("*** Could not execute '%s'\n", args[0]);
		exit(1);  
	}
	else {                                  
		while (wait(&currentstate) != pid)       
			;
	}

}


void PIPECOMMAND(char **token, int INput1)
{
	int pipefd[2];
	int pid;
	int arg[2] = { 0 };
	if (INput1 == 3){
		arg[0] = 1;
		arg[1] = 2;
	}
	else if (INput1 == 4){
		arg[0] = 2;
		arg[1] = 3;
	}
	else if (INput1 == 5){
		arg[0] = 3;
		arg[1] = 3;
	}

	char *catenate[arg[0]];
	char *GREP[arg[1]];
	
	int i = 0;
	int j = 0;
	if (INput1 == 4){
		catenate[j] = token[j];
		catenate[j+1] = NULL;
		GREP[j] = token[j+2];
		GREP[j+1] = token[j+3];
		GREP[j+2] = NULL;
	}
	else if (INput1 == 3){
		catenate[j] = token[j];
		catenate[j+1] = NULL;
		GREP[j] = token[j+2];
		GREP[j+1] = NULL;
	}
	else if (INput1 == 5){
		
		catenate[j] = token[j];
		catenate[j+1] = token[j+1];
		catenate[j+2] = NULL;
		GREP[j] = token[j+3];
		GREP[j+1] = token[j+4];
		GREP[j+2] = NULL;}
	pipe(pipefd);

	pid = fork();

	if (pid == 0)
	{
		dup2(pipefd[0], 0);
		close(pipefd[1]);
		execvp(GREP[0], GREP);
	}
	else
	{
		dup2(pipefd[1], 1);
		close(pipefd[0]);
		execvp(catenate[0], catenate);
	}
}



void  INThandler(int sig)
{
	char  c;

	signal(sig, SIG_IGN);
	printf("Did you hit Ctrl-C?\n"
		"Do you really want to quit? [y/n] ");
	c = getchar();
	if (c == 'y' || c == 'Y')
		exit(0);
	else
		signal(SIGINT, INThandler);
	getchar(); // Get new line character
}


void parser(){

	int i, j;
	char str[128];
	int SpaceCounter = 0;
	int ValuesMatch = 0;
	int tokenCheck = 0;
	int INput1 = 0;
	int row = 0;
	char key[] = { '<', '|', '>', '&' };
	char token[128][128];
	int SpecCharLoc;

	while (fgets(str, 50, stdin))
	{
		SpecCharLoc = 0;
		SpaceCounter = 0;
		ValuesMatch = 0;
		row = 0;
		INput2 = 0;
		tokenCheck = 0;
		fflush(stdout);
		
		for (i = 0; i<sizeof(str); i++){
			str[i] = '\0';
		}
		
		for (i = 0; i<INput2 + 1; i++){
			for (j = 0; j<strlen(key); j++){
				if (*token[i] == key[j]){
					printf("Special Token: %s\n", token[i]);
					SpecCharLoc = i;
					break;
				}
				else if (j == (3) && *token[i] != '\n'){
					printf("Token: %s\n", token[i]);
				}


			}
		}
		
		for (i = 0; i<INput2 + 1; i++){
			token[i] = token[i];
		}
		
		for (i = 0; i<strlen(str); i++){      
			for (j = 0; j<strlen(key); j++){  
				if (str[i] == ' '&& str[i] != '\n'){
					if (i != strlen(str) - 1){
						if (SpaceCounter == 0 && tokenCheck != 0){
							INput2++;
							row = 0;//reset
							SpaceCounter = 1;
							ValuesMatch = 0;
						}
					}
				}
				else if (str[i] == key[j]){//Checks for a ValuesMatching values
					if (str[i - 1] != ' '&& ValuesMatch == 0){
						if (i != 0){
							INput2++;//increment for next value
						}
					}
					row = 0;
					token[INput2][row] = str[i];
					if (i != (strlen(str) - 2)){
						INput2++;
					}
					ValuesMatch = 1;
					tokenCheck = 0;
					break;
				}

				else if (j == (3) && str[i] != '\n'){//stores value if not special.
					if (str[i] != ' '){
						token[INput2][row] = str[i];
						row++;//increment to go to next value
						SpaceCounter = 0;
						tokenCheck = 1;
						ValuesMatch = 0;
					}
				}
			}
		}
		char** token = (char**)malloc(INput2 + 1 * sizeof(char*));
		token[INput2 + 1][0] = '\0';
		
		if (SpecCharLoc >= 0 && *token[SpecCharLoc] == '|')
		{

			int currentstate;
			int process = fork();
			if (process == 0) {
				PIPECOMMAND(token, INput2 + 1);
				exit(0);
			}
			else if (process == -1){
				perror("failedforkingprocess\n");
				exit(1);
			}
			else {
				while ((process = wait(&currentstate)) != -1)
					;
			}

		}
		else if (SpecCharLoc >= 0 && *token[SpecCharLoc] == '>')
		{

			if (SpecCharLoc >= 1 && *token[SpecCharLoc - 1] == '&') 
			{
				APPEND_FUNC(token, token[SpecCharLoc + 1], 3);
			}
			else if (SpecCharLoc >= 1 && *token[SpecCharLoc - 1] == '>')
			{
				if (SpecCharLoc >= 2 && *token[SpecCharLoc - 2] == '2)
				{
					APPEND_FUNC(token, token[SpecCharLoc + 1], 1);
				}
				else // >>
				{
					APPEND_FUNC(token, token[SpecCharLoc + 1], 0);
				}
			}
			else if (SpecCharLoc >= 1 && *token[SpecCharLoc - 1] == '2') /
			{
				REDIRECT_FUNC(token, token[SpecCharLoc + 1], 2);
			}
			else if (SpecCharLoc >= 1 && *token[SpecCharLoc - 1] == '1') 
			{
				REDIRECT_FUNC(token, token[SpecCharLoc + 1], 1);
			}
			else{

				REDIRECT_FUNC(token, token[SpecCharLoc + 1], 1);

			}
		}
		else if (*token[SpecCharLoc] == '<'){ // <

			REDIRECT_FUNC(token, token[SpecCharLoc + 1], 0);
		}

		if (SpecCharLoc == 0){
			int currentstate;
			if ((currentstate = FORK_EXEC(INput2, token)) != 0){
				printf("Command %s returned %d.\n", token[0], currentstate);
			}
		}

		memset(token, '\0', sizeof(token[0][0]) * 128 * 128);
		
	}

}

int main(){
	signal(SIGINT, INThandler);
	parser();
	return 0;
}