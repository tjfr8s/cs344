#ifndef SMALLSH_H
#define SMALLSH_H

#define MAX_COMMAND_LENGTH 2048
#define MAX_ARGS 512

int     parseArguments(char* commandString, int maxArgs, int* numArgs, char** argArray);
void    freeArgArray(char** argArray, int numArgs);
int     tokenizeArguments(char* commandString, int maxArgs, int* numArgs, char** argArray);
void    getlineHandleSIGINT(int signum);
void    getInputString(char* stringBuffer);

#endif 
