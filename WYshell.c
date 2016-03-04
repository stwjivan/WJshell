#include <stdio.h>
#include <fcntl.h>
#include <sys/types.h>
#include <stdlib.h>
#include <unistd.h>
#include <pwd.h>
#include <string.h>
#include <sys/wait.h>
#include <errno.h>
#include <limits.h>
#include <signal.h>

#include "cmd_structure.h"
#include "command.h"

void print_prompt () {
	struct passwd *user;
	char buffer[PATH_MAX+1];
	char *cwd=getcwd(buffer,PATH_MAX+1);
	user=getpwuid(getuid());
	printf("%s@WYShell:%s$ ",user->pw_name,cwd);
}

int main () {
	char s[300];
	int status;
	int pid;
	struct Command command;
	while (1) {
		print_prompt();
		fgets(s,sizeof s, stdin);
		//check if there are finished child
		if ((pid=waitpid(-1,&status,WNOHANG))>0) {
			printf("[%d] finished\n", pid);
		}
		if (strcmp(s,"\n")==0) {
			continue; //if there is no input,skip reading command
		}
		ReadCommand(s,&command);
		ReadRedirectAndBackground(&command);
		int i;
		int fds[2];
		int pre_pipein=0;
		for (i=0;i<command.num_sub_commands;i++) {
			//check if the command is built-in command
			if (strcmp(command.sub_commands[i].argv[0],"cd")==0) {
				int err=chdir(command.sub_commands[i].argv[1]);
				if (err <0) perror("cd");
				break;
			}
			if(i<command.num_sub_commands-1) {
				int err=pipe(fds);
				if (err<0) perror("pipe");
			}
			int pid=fork();
			if (pid==0) {
				//child process
				if (command.sub_commands[i].hasPrefix==1) {
					int j=0;
					while(command.sub_commands[i].prefix[j]!=NULL) {
						int err=putenv(command.sub_commands[i].prefix[j]);
						if (err!=0) {
							perror("putenv");
							exit(1);
						}
						j++;
					}
				}
				if (command.num_sub_commands>1) {         //pipe operation, if there are more than one command
					dup2(pre_pipein,0);                   //change the stdin to accept data from previous process
					close(fds[0]);                        //close the unused pipe port
					if (i!=command.num_sub_commands-1) {  //except the last subcommand, change the stdout
					dup2(fds[1],1);
					}
				}
				//implement redirect-in, only the first command need to redirect
				if (i==0 && command.stdin_redirect!=NULL) { 	
					close(0);
					int fd=open(command.stdin_redirect,O_RDONLY);
					if (fd<0) {
						if (errno==ENOENT) {
							fprintf(stderr, "%s : File not found\n", command.stdin_redirect);
							exit(1);
						} else {
							perror(command.stdin_redirect);
							exit(1);
						}	
					}
				}
				//implement redirect-out, only the last subcommand need to redirect
				if (i==command.num_sub_commands-1 && command.stdout_redirect!=NULL) { 		
					close(1);
					int fd=open(command.stdout_redirect, O_WRONLY|O_CREAT,0660);
					if (fd<0) {
						if (errno==ENOENT) {
							fprintf(stderr, "%s : Cannot create file\n", command.stdout_redirect);
							exit(1);
						} else {
							perror(command.stdout_redirect);
							exit(1);
						}
					}
				}
				//execution of the subcommand
				if(execvp(command.sub_commands[i].argv[0],command.sub_commands[i].argv)<0) {
					if (errno==ENOENT) {
						fprintf(stderr, "%s : Command not found\n", command.sub_commands[i].argv[0]);
					} else {
						perror(command.sub_commands[i].argv[0]);
					}
				}
			} else {
				//parent process
				if (command.num_sub_commands>1) {
					close(fds[1]);     //close the unused pipe port
					pre_pipein=fds[0]; //save the pipe input for next fork	
				}	
				//print the pid if running in the background
				if (command.background==0) {
					wait(&status);
				} else {
					if (i==command.num_sub_commands-1) {
						printf("[%d]\n", pid);
					}
				}
			}
		}
	} 
}