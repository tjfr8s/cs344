#define _POSIX_C_SOURCE 1
#define _GNU_SOURCE
#include <stdio.h>
#include <errno.h>
#include <dirent.h>
#include <stdbool.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/wait.h>
#include "smallsh.h"
// Prompt user for input and recieve this input.

// Parse user input.
// - expand $$ to pid

// Handle built-in commands

// Handle non-buit-in commands
// - fork()
// - dup2() redireciton 
// - exec()
//  - set status on failed exec
//  - terminate process on failed exec
//  - use path to look for non-bulit-in commands (error and status 1 on fail)
// - wait for completion of foreground commands before prompting user (waitpid())
// - periodically check to see if background processes are finished with (waitpid(..NOHANG..))
// store pids of background processes in array while they are running
// - background proccesses redirect dev/null via stdin and stdout if no other options given
// - print process id of bg proc when started
// - on term, print proc id and exit status (check for completed procs just
// before prompting for new command. print this then.)
// - SIGINT should only term fg process, not shell (use sigaction())
// - children term self on sig int (parent doesn't term them) parent should
// immediately print number of sig that killed child
// - dont term bg with SIGINT
// - builtins ingnore background processes
//

// Redirect(to, from, inOrOut)
// - allow redireciton of input or output
// - open input files as wonly and output as ronly 
// - error and status 1 on fail
int main(int argc, char** argv) {
    size_t              maxCommandStringLength = MAX_COMMAND_LENGTH + 1;
    char                commandString[maxCommandStringLength];
    char*               argArray[MAX_ARGS + 1] = {};
    int                 numArgs = 0;
    int                 prevExitStatus = 0;
    struct sigaction    SIGINT_action = {{0}};
    SIGINT_action.sa_handler = shellHandleSIGINT;
    SIGINT_action.sa_flags = SA_RESTART;
    sigfillset(&SIGINT_action.sa_mask);
    sigaction(SIGINT, &SIGINT_action, NULL);

    while(1) {
        memset(&commandString, 0, maxCommandStringLength);
        getInputString(commandString);
        printf("The input string: %s\n", commandString);
        fflush(stdout);

        parseArguments(commandString, MAX_ARGS, &numArgs, argArray, &prevExitStatus);
        freeArgArray(argArray, &numArgs);
    }

    return 0;
}

void lsBuiltIn() {
    struct dirent*  entry;
    DIR*            dr = opendir(".");

    if (dr == NULL) {
        perror("couldn't open dir");
        return;
    }

    while ((entry = readdir(dr)) != NULL) {
        printf("%s ", entry->d_name);
        fflush(stdout);
    }

    closedir(dr);
    printf("\n");
    fflush(stdout);
    return;
}

bool isBuiltIn(char* command) {
    char*   builtIns[4] = {"exit", "ls", "cd", "status"};
    int     i;
    for(i = 0; i < 4; i++) {
        if (!strcmp(command, builtIns[i])) {
            return true;
        }
    }
    return false;
}

bool hasValidNumArgs(char** argArray, 
                     int numArgs, 
                     int maxArgs, 
                     int minArgs, 
                     bool isBuiltIn) {
    if (numArgs < minArgs) {
        perror("too few arguments\n");
        return false;
    } else if ((numArgs == maxArgs + 1) 
            && isBuiltIn 
            && !strcmp(argArray[numArgs - 1], "&")) {
        return true; 
    } else if (numArgs > maxArgs) {
        perror("too many arguments\n");
        return false;
    } else {
        return true;
    }
}

void executeBuiltIn(char** argArray, int numArgs, int prevExitStatus) {
    if (!strcmp("ls", argArray[0]) 
            && hasValidNumArgs(argArray, numArgs, 1, 1, true)) {
        lsBuiltIn();
                 
    } else if (!strcmp("exit", argArray[0])
            && hasValidNumArgs(argArray, numArgs, 1, 1, true)) {
        freeArgArray(argArray, &numArgs);
        exit(0);
    } else if (!strcmp("cd", argArray[0])
           && hasValidNumArgs(argArray, numArgs, 2, 1, true)) {
        char* path = NULL;
        if (numArgs == 1) {
            chdir(getenv("HOME"));
        } else {
            path = argArray[1];
            chdir(path);
        }
        
    } else if (!strcmp("status", argArray[0])
           && hasValidNumArgs(argArray, numArgs, 1, 1, true)) {
        printf("status: %d\n", prevExitStatus);
    } 
}

void executeCommand(char** argArray, int* numArgs, int* prevExitStatus) {
    int i;
    for (i = 0; i < *numArgs; i++) {
        printf("arg%d: %s\n", i, argArray[i]);
    }
    fflush(stdout);
    pid_t               pid;
    int                 status = -5;
    struct sigaction    SIGINT_action_child = {{0}};
    bool                isBackground = false;

    SIGINT_action_child.sa_handler = childHandleSIGINT;
    SIGINT_action_child.sa_flags = SA_RESTART;
    sigfillset(&SIGINT_action_child.sa_mask);

    if (!strcmp(argArray[*numArgs - 1], "&")) {
        free(argArray[*numArgs - 1]);
        argArray[*numArgs - 1] = NULL;
        (*numArgs)--;
        isBackground = true;
    }

    pid = fork();
    switch(pid) {
        case -1:
            perror("\nerror creating new process\n");
        case 0:
            // Child process, execute command
            // Set appropraite handler funciton for signals.
            sigaction(SIGINT, &SIGINT_action_child, NULL);
            if (execvp(argArray[0], argArray) > 0) {
                exit(1);
            }
        default:
            printf("waiting\n");
            fflush(stdout);
            if (waitpid(pid, &status, 0) == -1) {
                printf("\nwaitpid error %s\n", strerror(errno));
            }
            printf("status: %d\n", status);
            if (WIFEXITED(status) != 0) {
                printf("finished waiting | exit status: %d\n", WEXITSTATUS(status));
                *prevExitStatus = WEXITSTATUS(status);
            } else {
                printf("finished waiting | signal value: %d\n", WTERMSIG(status));
                *prevExitStatus = WTERMSIG(status);
            }
            fflush(stdout);
    }
}

int parseArguments(char* commandString, 
                  int maxArgs, 
                  int* numArgs, 
                  char** argArray,
                  int* prevExitStatus) {
    if (tokenizeArguments(commandString, maxArgs, numArgs, argArray) == -1) {
        perror("error tokenizing argument string\n");
        return -1;
    }

    if (isBuiltIn(argArray[0])) {
        executeBuiltIn(argArray, *numArgs, *prevExitStatus);
    } else  {
        executeCommand(argArray, numArgs, prevExitStatus);
    
    }
    return 0;
}

void freeArgArray(char** argArray, int* numArgs) {
    int curArg = 0;
    for(curArg = 0; curArg < *numArgs; curArg++) {
        free(argArray[curArg]);
        argArray[curArg] = NULL;
    }
    *numArgs = 0;
}

/*******************************************************************************
 * Description: This function takes the user's command string and tokenizes
 * it into individual arguments delimited by spaces. These arguments are 
 * stored in the passed argArray and the total number of arguments is stored
 * in numArgs.
 *
 * @param   char*   commandString   the string of argus to be parsed.
 * @param   int     maxArgs         the max number of args allowed.
 * @param   int*    numArgs         the number of arguments (pass back to user).
 * @param   char**  argArray        the array of arguments to be populated.
*******************************************************************************/
int tokenizeArguments(char* commandString, 
                  int maxArgs, 
                  int* numArgs, 
                  char** argArray) {
    char*   token = NULL;
    char    delim = ' ';

    token = strtok(commandString, &delim);
    while (token != NULL) {
        if (*numArgs == maxArgs) {
            perror("Too many arguments");
            return -1;
        }

        if((argArray[*numArgs] = 
                    malloc(sizeof(char) * (strlen(token) + 1))) == NULL) {
            perror("error allocating memory for arg array");
            return -1;
        }

        strcpy(argArray[*numArgs], token); 
        (*numArgs)++;
        token = strtok(NULL, &delim); 
    }

    return 0;
}

void shellHandleSIGINT(int signum) {
    char* message = "\nSIGINT received\n";
    write(STDOUT_FILENO, message, strlen(message) + 1);
}

void childHandleSIGINT(int signum) {
    char* message = "\nChild SIGINT received\n";
    write(STDOUT_FILENO, message, strlen(message) + 1);
    _exit(signum);
}

/*******************************************************************************
 * Description: This function gets input from the user of no more than
 * MAX_COMMAND_LENGTH characters. It is robust to singnal interruption.
 *
 * @param   char*   commandString   A buffer in which to store the user's 
 *                                  command.
*******************************************************************************/
void getInputString(char* commandString) {

    size_t      bufferSize = 0;
    char*       inputBuffer = NULL;
    int         numRead = 0;


    while(1) {
        printf("Enter a command: ");
        fflush(stdout);
        numRead = getline(&inputBuffer, &bufferSize, stdin);
        if (numRead == -1) {
            clearerr(stdin);
        } else if (numRead > MAX_COMMAND_LENGTH + 1) {
            free(inputBuffer);
            inputBuffer = NULL;
        } else {
            break;
        }
    }

    inputBuffer[strcspn(inputBuffer, "\n")] = '\0';
    strcpy(commandString, inputBuffer);
    free(inputBuffer);
    inputBuffer = NULL;
}
