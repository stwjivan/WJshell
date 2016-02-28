#include <stdio.h>
#include <fcntl.h>
#include <sys/types.h>
#include <stdlib.h>
#include <unistd.h>
#include <pwd.h>
#include <string.h>
#include <sys/wait.h>

#include "cmd_structure.h"
#include "command.h"

void print_prompt () {
	struct passwd *user;
	user=getpwuid(getuid());
	printf("%s@WJShell:$ ",user->pw_name);
}

int main () {
	char s[300];
	while (1) {
		print_prompt();
		fgets(s,sizeof s, stdin);
		struct Command command;
		ReadCommand(s,&command);
		ReadRedirectAndBackground(&command);
		//struct SubCommand *last=&(command.sub_commands[command.num_sub_commands-1]); 
		int i;
		int fds[2];
		int pre_pipein=0;
		for (i=0;i<command.num_sub_commands;i++) {
			if(i<command.num_sub_commands-1) {
				int err=pipe(fds);
				if (err<0) perror("pipe");
			}
			int pid=fork();
			if (pid==0) {
				//child process
				dup2(pre_pipein,0); //change the stdin to accept data from previous process
				if (i==0 && command.stdin_redirect!=NULL) {
					close(0);
					int fd=open(command.stdin_redirect,O_RDONLY);
					if (fd<0) {
						perror(command.stdin_redirect);
						exit(1);	
					}
				}
				if (i==command.num_sub_commands-1 && command.stdout_redirect!=NULL) {
					close(1);
					int fd=open(command.stdout_redirect, O_WRONLY|O_CREAT,0660);
					if (fd<0) {
						perror(command.stdout_redirect);
						exit(1);
					}
				}

				if (i!=command.num_sub_commands-1) {
					dup2(fds[1],1);
				}
				close(fds[0]);
				if(execvp(command.sub_commands[i].argv[0],command.sub_commands[i].argv)<0) {
					perror(command.sub_commands[i].argv[0]);
				}
			} else {
				//parent process
				//close the unused pipe port
				close(fds[1]);
				pre_pipein=fds[0];
				//print the pid if running in the background
				if(command.background==1 && i==command.num_sub_commands-1) {
					printf("[%d]\n", pid);					
				}
			}
		}
		if (command.background==0) {
			//if not running in the background, wait for all childs
			while(wait(NULL)>0);
		}
	} 

}