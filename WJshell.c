#include <stdio.h>
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
//test

int main () {
	char s[300];
	while (1) {
		print_prompt();
		fgets(s,sizeof s, stdin);
		struct Command command;
		ReadCommand(s,&command);
		ReadRedirectAndBackground(&command);
		struct SubCommand *last=&(command.sub_commands[command.num_sub_commands-1]); 
		int ret=fork();
		if (ret==0) {
			if(execvp(last->argv[0],last->argv)<0) {
				perror(last->argv[0]);
			}
		} else {
			int w=wait(NULL);
		}
	} 

}