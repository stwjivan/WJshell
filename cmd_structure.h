#ifndef CMD_STRUCTURE_H
#define CMD_STRUCTURE_H
#define MAX_SUB_COMMANDS    5
#define MAX_ARGS            10

struct SubCommand {
    char *line;
    char *argv[MAX_ARGS];
    char *prefix[MAX_ARGS];
    int hasPrefix;
};

struct Command {
    struct SubCommand sub_commands[MAX_SUB_COMMANDS];
    int num_sub_commands;
    char *stdin_redirect;
    char *stdout_redirect;
    int background; 
};

#endif