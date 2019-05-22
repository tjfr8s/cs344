#ifndef SMALLSH_H
#define SMALLSH_H

#define MAX_COMMAND_LENGTH 2048
#define MAX_ARGS 512
#define MAX_BG_PROCS 4096

void    redirection(char** argArray, int* numArgs, int* redirSTDIN, int* redirSTDOUT);
void    checkBackgroundProcs(int* inBackground, int* numInBackground);
void    lsBuiltIn();
bool    isBuiltIn(char* command);
bool    hasValidNumArgs(char** argArray, int numArgs, int maxArgs, int minArgs, bool isBuiltIn);
void    executeBuiltIn (char** argArray, int numArgs, int prevExitStatus, int* inBackground, int* numInBackground);
void    executeCommand(char** argArray, int * numArgs, int* prevExitStatus, int* inBackground, int* numInBackground);
int     parseArguments(char* commandString, int maxArgs, int* numArgs, char** argArray, int* prevExitStatus, int* inBackground, int* numInBackground);
void    freeArgArray(char** argArray, int* numArgs);
int     tokenizeArguments(char* commandString, int maxArgs, int* numArgs, char** argArray);
void    shellHandleSIGINT(int signum);
void    childHandleSIGINT(int signum);
void    getInputString(char* stringBuffer);

#endif 
