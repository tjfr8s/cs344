/*******************************************************************************
 * Author: Tyler Freitas
 * Date: 190526
 * Description: This program implements a shell through which build-in commands
 * and external programs can be executed. The shell allows for input redirection
 * using the < and > characters. A process can be run in the background using
 * the & character at the end of a command. Background processes can be disabled
 * by sending a SIGTSTP, and re-enabled by sending a SIGTSTP again.
*******************************************************************************/
#ifndef SMALLSH_H
#define SMALLSH_H

// Maxium supported command length in characters.
#define MAX_COMMAND_LENGTH 2048
// Maxium number of supported command arguments.
#define MAX_ARGS 512
// Maxium number of supported background processes.
#define MAX_BG_PROCS 4096

void    redirection(char** argArray, 
                    int* numArgs, 
                    int* redirSTDIN, 
                    int* redirSTDOUT);
void    checkBackgroundProcs(int* inBackground, 
                             int* numInBackground);
bool    isBuiltIn(char* command);
bool    hasValidNumArgs(char** argArray, 
                        int numArgs, 
                        int maxArgs, 
                        int minArgs, 
                        bool isBuiltIn);
void    executeBuiltIn (char** argArray, 
                        int numArgs, 
                        int prevExitStatus, 
                        int* inBackground, 
                        int* numInBackground);
void    executeCommand(char** argArray, 
                       int * numArgs, 
                       int* prevExitStatus, 
                       int* inBackground, 
                       int* numInBackground);
int     parseArguments(char* commandString, 
                       int maxArgs, 
                       int* numArgs, 
                       char** argArray, 
                       int* prevExitStatus, 
                       int* inBackground, 
                       int* numInBackground);
void    freeArgArray(char** argArray, int* numArgs);
int     tokenizeArguments(char* commandString, 
                          int maxArgs, 
                          int* numArgs, 
                          char** argArray);
void    shellHandleSIGINT(int signum);
void    shellHandleSIGTSTP(int signum);
void    childHandleSIGINT(int signum);
void    expandPID(char* commandString);
void    getInputString(char* stringBuffer);
int     isComment(char* arg1);

#endif 
