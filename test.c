#include <stdio.h>
#include <sys/types.h>
#include <stdlib.h>
#include <unistd.h>
#include <pwd.h>
#include <string.h>


extern char**environ;
int main () {
	char *entry_env=*environ;
	int i;
	for(i=1;entry_env;i++) {
		printf("%s\n", entry_env);
		entry_env=*(environ+i);
	}
}