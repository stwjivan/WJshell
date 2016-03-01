#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "cmd_structure.h"
#include "command.h"

void ReadArgs(char *in, char **argv, int size) {
    char * dup_in;
    dup_in=strdup(in);
    char *p;
    p=strtok(dup_in," \n"); //two dilms: space & \n
    int i=0;
    while(p!=NULL&&i<size-1) {
        argv[i]=p;
        p=strtok(NULL," \n");
        i++;
    }
    argv[i]=NULL;
}


void ReadCommand(char *line, struct Command *command) {
    //change the \n with \0
    char *dup_line=strdup(line);
    dup_line[strlen(dup_line)-1]='\0';
    //split the line into subcommands
    char *p;
    p=strtok(dup_line,"|");
    int i=0;
    while (p!=NULL&&i<MAX_SUB_COMMANDS) {
        char *dup_token=strdup(p);
        command->sub_commands[i].line=dup_token;
        p=strtok(NULL,"|");
        i++;
    } 
    command->num_sub_commands=i;
    //run ReadArgs for each subcommands
    int j;
    for(j=0;j<command->num_sub_commands;j++) {
        ReadArgs(command->sub_commands[j].line, command->sub_commands[j].argv, MAX_ARGS);
        ReadPrefix(&(command->sub_commands[j]));
    }
}

void ReadPrefix(struct SubCommand *subcommand) {
	int i=0;
	//find if there is a '=' in argv[i]
	while (subcommand->argv[i]!=NULL && strchr(subcommand->argv[i],'=')!=NULL && *(subcommand->argv[i])!='=') {
		subcommand->prefix[i]=strdup(subcommand->argv[i]);
		i++;
	}
	subcommand->prefix[i]=NULL;
	if (i==0) {
		subcommand->hasPrefix=0;
	} else {
		subcommand->hasPrefix=1;
		int j=0;
		while (subcommand->argv[i]!=NULL) {
			subcommand->argv[j]=subcommand->argv[i];
			j++;
			i++;
		}
	subcommand->argv[j]=NULL;
	}
}

void ReadRedirectAndBackground(struct Command *command) {
	//set new fields to default value;
	command->background=0;
	command->stdin_redirect=NULL;
	command->stdout_redirect=NULL;
	//a pointer to the last subcomand
	struct SubCommand *last=&(command->sub_commands[command->num_sub_commands-1]); 
	// Get the length of argv
	int i=0;
	while(last->argv[i]!=NULL) {
		i++;
	}
	//go through the arguments in reverse order
	for (i=i-1;i>0;i--) {
		if (strcmp(last->argv[i],"&")==0) {
			if (last->argv[i+1]==NULL) {
				command->background=1;
			}
			last->argv[i]=NULL;
		} else if (strcmp(last->argv[i],"<")==0) {
			if (last->argv[i+1]==NULL) {
				last->argv[i]=NULL;
			} else {
				if (strcmp(last->argv[i+1],"<")!=0||strcmp(last->argv[i+1],">")!=0||strcmp(last->argv[i+1],"&")!=0) {
					command->stdin_redirect=last->argv[i+1]; //< file pattern		
				}
				last->argv[i]=NULL;	
			}				
		} else if (strcmp(last->argv[i],">")==0) {
			if (last->argv[i+1]==NULL) {
				last->argv[i]=NULL;
			} else {
				if (strcmp(last->argv[i+1],"<")!=0||strcmp(last->argv[i+1],">")!=0||strcmp(last->argv[i+1],"&")!=0) {
					command->stdout_redirect=last->argv[i+1]; //> file pattern		
				}
				last->argv[i]=NULL;	
			}		
		} else {
			continue;
		}
	}
}
