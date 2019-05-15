#ifndef SMALLSH_H
#define SMALLSH_H

#define MAX_COMMAND_LENGTH 2048
#define MAX_ARGS 512

void getlineHandleSIGINT(int signum);
int getInputString(char* stringBuffer);

#endif 
