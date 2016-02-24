#ifndef COMMAND_H
#define COMMAND_H

#include "cmd_structure.h"

void ReadArgs(char *in, char **argv, int size);
void ReadCommand(char *line, struct Command *command);
void ReadRedirectAndBackground(struct Command *command);

#endif