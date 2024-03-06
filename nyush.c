/*
Acknowledgements:
    https://man7.org/linux/man-pages/man3/basename.3.html 
    https://youtube.com/watch?v=5fnVr-zH-SE
    https://man7.org/linux/man-pages/man2/open.2.html
    https://zestedesavoir.com/tutoriels/755/le-langage-c-1/1043_aggregats-memoire-et-fichiers/4279_structures/
    https://man7.org/linux/man-pages/man2/signal.2.html#RETURN_VALUE
    https://www.youtube.com/watch?v=OVFEWSP7n8c
    https://man7.org/linux/man-pages/man2/pipe.2.html
    https://linux.die.net/man/2/waitpid
    https://man7.org/linux/man-pages/man2/kill.2.html
    https://stackoverflow.com/questions/50610781/implementing-pipe-and-redirection-together-in-c
    https://stackoverflow.com/questions/47478392/redirection-at-the-end-of-the-pipe-c-shell
    https://stackoverflow.com/questions/8082932/connecting-n-commands-with-pipes-in-a-shell
*/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdbool.h>
#include <string.h>
#include <libgen.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <errno.h>
#include <fcntl.h>

// struct Job{ //store job info into jobArr
//     int index;
//     char cmd[1000];
// };
// struct Job jobArr[100]; 
int currJobs = 0; //num of jobs curr suspended

void handleErr(int errCode){
    switch (errCode){
        case 1: 
            fprintf(stderr, "Error: invalid command\n");
            break;
        case 2:
            fprintf(stderr, "Error: invalid program\n");
            break;
        case 3:
            fprintf(stderr, "Error: invalid directory\n");
            break;
        case 4:
            fprintf(stderr, "Error: invalid job\n");
            break;
        case 5:
            fprintf(stderr, "Error: there are suspended jobs\n");
            break;
        case 6:
            fprintf(stderr, "Error: invalid file\n");
            break;
    }
}


void handleSignal(int sig){
    if (sig == SIGINT || sig == SIGQUIT){
        // fprintf(stdout, "\n");
        //do nothing as it's been handled  
    }
}

// void handleChSignal(int sig){
//     if (sig == SIGINT){
//         if (chPid>0){ 
//             addJob(chPid, currCommand);
//         } 
//         kill(chPid, sig); 
//     }
// }

void handleIO(char *args[], int direction, char* in, char* out) {
    pid_t pid = fork();
    // int i = 0;
    // while (args[i] != NULL){
    //     printf("%d: %s\n", i, args[i]);
    //     i++;
    // }

    if(pid==-1){
		exit(1);
	}
    //1,2,3: in, out, append out respectively
    else if(pid==0){
        if (direction == 1){
            int file = open(in, O_RDONLY, 0777); 
            if (file == -1) {
                handleErr(6);
                exit(1); 
        
            } 
			dup2(file, STDIN_FILENO);
			close(file);
        }
		else if (direction == 2){
			int file = open(out, O_WRONLY | O_CREAT | O_TRUNC, 0777); 
			dup2(file, STDOUT_FILENO); 
			close(file);
		}

        else if (direction == 3){
			int file = open(out, O_WRONLY | O_CREAT | O_APPEND, 0777); 
			dup2(file, STDOUT_FILENO); 
			close(file);
		}
        
		int err = execvp(args[0], args);
		if (err == -1){
			kill(getpid(),SIGTERM);
		}		 
	}
	waitpid(pid,NULL,0);
}


void handlePipe(char* args[]){
    //needs work...
	char *cmd[1000];
	bool term = 0;
	int i = 0, j = 0, cmdCount = 0;
    while (args[i] != NULL){
		if (strcmp(args[i],"|") == 0){
			cmdCount++;}
		i++;
	}

	cmdCount+=1;
    i = 0;
	while (term == 0 && args[j] != NULL){
        int fd1[2], fd2[2], k = 0;
		while (strcmp(args[j],"|") != 0){
			cmd[k] = args[j];
			j++;	
			if (args[j] == NULL){
				k+=1;
                term = 1;
				break;
			}
			k++;
		}
		cmd[k] = NULL;
		j++;

        //n-1 pipes, determine order of curr command & if pipe before, after, or on both sides 
		if (i % 2 == 0){
			pipe(fd2); 
		}
        else{
			pipe(fd1); 
		}
		pid_t pid=fork();
		if(pid==-1){	
            //fail to fork
            close(fd1[1]);		
			close(fd2[1]);			
			exit(1);
		}

		if(pid==0){
            //in child for exec
			if (i+1 == cmdCount){
				if (cmdCount % 2 == 0){ 
					dup2(fd2[0],STDIN_FILENO);
				}
                else{ 
					dup2(fd1[0],STDIN_FILENO);
				}
			}
            else if (i == 0){ 
				dup2(fd2[1], STDOUT_FILENO);
			}
            else{ 
				if (i % 2 == 0){
					dup2(fd1[0],STDIN_FILENO); 
					dup2(fd2[1],STDOUT_FILENO);
				}
                else{ 
					dup2(fd2[0],STDIN_FILENO); 
					dup2(fd1[1],STDOUT_FILENO);					
				} 
			}
			if (execvp(cmd[0],cmd)==-1){
				exit(1);
			}		
		}	
		
        //fix...
		if (i+1 == cmdCount){
			if (cmdCount % 2 == 0){					
				close(fd2[0]);
			}
            else{					
				close(fd1[0]);
			}
		}
        else{
			if (i % 2 == 0){					
				close(fd1[0]);
				close(fd2[1]);
			}
            else{					
				close(fd2[0]);
				close(fd1[1]);}
		}		
		waitpid(pid,NULL,0);		
		i++;	
	}
}


void handleFG(char* args[]) {
    //wrong
    if (args != NULL){
        handleErr(4);
        exit(1);
    }
}

void handleExternal(char *args[]) {
    pid_t pid = fork();
    if (pid == -1) {
        return;
    }
    else if (pid == 0) {
        // chPid = getpid();
        // signal(SIGTSTP, handleChSignal);

        if (strchr(args[0], '/') != NULL) {    
            if (execvp(args[0], args) == -1) {
                //exec fail (cmd not in /usr/bin/)
                handleErr(2);
                exit(1); 
            }
        }
        else{
            // "/" detected so get path 
            char *cmdPath = (char *)malloc(strlen("/usr/bin/") + strlen(args[0]) + 1);
            if (cmdPath != NULL) {
                strcpy(cmdPath, "/usr/bin/");
                strcat(cmdPath, args[0]);
                if (execv(cmdPath, args) == -1) {
                    handleErr(2); 
                    free(cmdPath);
                    exit(1);
                }
                free(cmdPath);
            } 
            else {
                exit(1);
            }
        }
    } 

    else {
        waitpid(pid, NULL, 0);
    }
}


void handleCD(char* args[], int count){
	if (args[1] == NULL || count >= 3) {
        handleErr(1);
	}
	else{ 
		if (chdir(args[1]) == -1) {
            handleErr(3);
		}
	}
}

//NEEDS WORK 
void handleCmd(char *input) { 
    char *args[1000]; //idk bout this
    char *args2[1000]; //for pipe and i/o
    char *token = strtok(input, " ");
    int argCount = 0;

    // strncpy(currCommand, input, sizeof(currCommand) - 1);
    while (token != NULL) {
        args[argCount] = token;
        argCount++;
        token = strtok(NULL, " ");     
    }
    args[argCount] = NULL;


    for (int i = 0; i < argCount; i++){
		if ( (strcmp(args[i],">") == 0) || (strcmp(args[i],"<") == 0) || (strcmp(args[i],">>") == 0)){break;}
		args2[i] = args[i];
        // printf("%d : %s\n", i, args2[i]);
	}

    if (argCount == 0) { //whitespace input
        return;}

    //still needs to wait so no zombs tho
    if (strcmp(args[0], "exit") == 0) {
        if (argCount == 1) {
            if (currJobs != 0){
                handleErr(5);
            }
            else{
                exit(0);
            } 
        }
        else {
            handleErr(1);
            return;}
    } else if (strcmp(args[0], "cd") == 0) {
        // printf("CD cmd\n");
        // printf("%d\n", argCount);
        handleCD(args, argCount);
        return;
    } 

    else if (strcmp(args[0], "jobs") == 0) {
        if (args[1] != NULL){
            handleErr(1);
            return;
        }
        return;
    } 

    else if (strcmp(args[0], "fg") == 0) {
        if (argCount != 2){
            handleErr(1);
            return;
        }
        // handleFG(args);
        return;
    }

    else {
        int i = 0;
        while (args[i] != NULL){
            int j = i+1;
            if (strcmp(args[i],"<") == 0){
				if (args[j] == NULL || j == 1){
					handleErr(1);
					return;
				}

                if (strcmp(args[j],"<") == 0){
                    handleErr(1);
					return;
                }

                if (args[j+1] != NULL){
                    if( strcmp(args[j+1],"<") == 0){
                        handleErr(1);
					    return;
                    }
                }
                
                handleIO(args2, 1, args[j], NULL);
				return;
            }

            else if (strcmp(args[i],">") == 0){
				if (args[j] == NULL || j == 1){
					handleErr(1);
					return;
				}
				handleIO(args2, 2, NULL, args[j]);
				return;
			}

            else if (strcmp(args[i],">>") == 0){
				if (args[j] == NULL || j == 1){
					handleErr(1);
					return;
				}
                // printf("handled here\n");
				handleIO(args2, 3, NULL, args[j]);
				return;
			}

            else if (strcmp(args[i],"|") == 0){
                if (args[j] == NULL || j == 1){
					handleErr(1);
					return;
				}
				handlePipe(args);
				return;
            }

            else if (strcmp(args[i],"<<") == 0){
                handleErr(1);
				return;
            }

            i++;

        }
        handleExternal(args);
        return;
    }
}


int main() {
    char *currDir = NULL; 
    signal(SIGINT, handleSignal);
    signal(SIGQUIT, handleSignal);
    //signal(SIGCHLD, handleChSignal);

    while (true) {
        currDir = getcwd(NULL, 0);
        // printf("curr pid: %d\n", getpid());
        char *baseDir = NULL;
        baseDir = basename(currDir); 
        // printf("curr_dir: %s\n base_dir: %s\n", currDir, baseDir);    // check curr dir
        fprintf(stdout, "[nyush %s]$ ", baseDir);
        fflush(stdout);
        free(currDir);
        char input[1000];  //assume cd has no more than 1000 characters
        if (fgets(input, sizeof(input), stdin) == NULL) {
            printf("err\n");
            break;
        }
        size_t input_length = strlen(input);
        
        if (input_length > 0 && input[input_length - 1] == '\n') {
            input[input_length - 1] = '\0';
        }
        handleCmd(input);  //parse command func
    }
    return 0;
}

