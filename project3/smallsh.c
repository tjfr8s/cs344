#define _POSIX_C_SOURCE 1
#define _GNU_SOURCE
#include <stdio.h>
#include <errno.h>
#include <dirent.h>
#include <stdbool.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdbool.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/wait.h>
#include "smallsh.h"
static bool backgroundEnabled = true;
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
    char                commandString[maxCommandStringLength + 4];
    char*               argArray[MAX_ARGS + 1] = {};
    int                 numArgs = 0;
    int                 prevExitStatus = 0;
    int                 inBackground[MAX_BG_PROCS] = {0};
    int                 numInBackground = 0;
    struct sigaction    SIGINT_action = {{0}};
    struct sigaction    SIGTSTP_action = {{0}};

    SIGINT_action.sa_handler = SIG_IGN;
    SIGINT_action.sa_flags = SA_RESTART;
    sigfillset(&SIGINT_action.sa_mask);
    sigaction(SIGINT, &SIGINT_action, NULL);

    SIGTSTP_action.sa_handler = shellHandleSIGTSTP;
    SIGTSTP_action.sa_flags = SA_RESTART;
    sigfillset(&SIGTSTP_action.sa_mask);
    sigaction(SIGTSTP, &SIGTSTP_action, NULL);

    while(1) {
        if (numInBackground > 0) {
            checkBackgroundProcs(inBackground, &numInBackground);
        }
        memset(&commandString, 0, maxCommandStringLength);
        getInputString(commandString);
        expandPID(commandString);
        fflush(stdout);

        parseArguments(commandString, MAX_ARGS, &numArgs, argArray, &prevExitStatus, inBackground, &numInBackground);
        freeArgArray(argArray, &numArgs);
    }

    return 0;
}


void checkBackgroundProcs(int* inBackground, int* numInBackground) {
    int status = 0;
    int waitVal = 0;
    int i;
    for (i = 0; i < MAX_BG_PROCS; i++) {
        if (inBackground[i] > 0) {
            fflush(stdout);
            if ((waitVal = waitpid(inBackground[i], &status, WNOHANG)) == -1 && errno != 10) {
                printf("\nwaitpid error %s\n", strerror(errno));
                fflush(stdout);
            } else if (waitVal != 0) {
                (*numInBackground)--;
                if (WIFEXITED(status) != 0) {
                    printf("process id: %d, exit status: %d\n", inBackground[i], WEXITSTATUS(status));
                    fflush(stdout);
                } else {
                    printf("process id: %d, exit signal: %d\n", inBackground[i], WTERMSIG(status));
                    fflush(stdout);
                }
                inBackground[i] = 0;
            }
        }
    } 
}


bool isBuiltIn(char* command) {
    fflush(stdout);
    char*   builtIns[3] = {"exit", "cd", "status"};
    int     i;
    for(i = 0; i < 3; i++) {
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

void executeBuiltIn(char** argArray, int numArgs, int prevExitStatus, int* inBackground, int* numInBackground) {
                 
    if (!strcmp("exit", argArray[0])
            && hasValidNumArgs(argArray, numArgs, 1, 1, true)) {
        freeArgArray(argArray, &numArgs);

        int i;
        for (i = 0; i < MAX_BG_PROCS; i++) {
            if (inBackground[i] != 0) {
                printf("kill %d\n", inBackground[i]);
                fflush(stdout);
                kill(inBackground[i], SIGKILL);
            }
        }

        exit(0);
    } else if (!strcmp("cd", argArray[0])
           && hasValidNumArgs(argArray, numArgs, 3, 1, true)) {
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
            
        fflush(stdout);
    } 
}

void executeCommand(char** argArray, int* numArgs, int* prevExitStatus, int* inBackground, int* numInBackground) {
    fflush(stdout);
    int                 status = -5;
    bool                isBackground = false;
    struct sigaction    SIGINT_action_child = {{0}};
    struct sigaction    SIGTSTP_action_child = {{0}};
    int redirSTDIN  = 0;
    int redirSTDOUT = 0;
    int argIndex = 0;
    int infile = 0;
    int outfile = 0;
    pid_t pid;

    SIGINT_action_child.sa_handler = childHandleSIGINT;
    SIGINT_action_child.sa_flags = SA_RESTART | SA_NODEFER;
    sigfillset(&SIGINT_action_child.sa_mask);

    SIGTSTP_action_child.sa_handler = SIG_IGN;
    SIGTSTP_action_child.sa_flags = SA_RESTART | SA_NODEFER;
    sigfillset(&SIGTSTP_action_child.sa_mask);

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
            sigaction(SIGTSTP, &SIGTSTP_action_child, NULL);
            for (argIndex = 0; argIndex < *numArgs; argIndex++) {
                if(strcmp(argArray[argIndex], "<") == 0 
                        && argIndex < (*numArgs - 1)) {
                    redirSTDIN = 1;
                    infile = open(argArray[argIndex + 1], O_RDONLY);
                    if (infile == -1) {
                        exit(1);
                    }

                    dup2(infile, STDIN_FILENO);

                    int j = argIndex;
                    while(j < (*numArgs - 2)) {
                       char* temp = argArray[j]; 
                       argArray[j] = argArray[j + 2];
                       argArray[j + 2] = temp;
                       j++;
                    }

                    free(argArray[*numArgs - 1]);
                    argArray[*numArgs - 1] = NULL;
                    free(argArray[*numArgs - 2]);
                    argArray[*numArgs - 2] = NULL;
                    (*numArgs) = *numArgs - 2;
                    argIndex = argIndex - 1;
                    fflush(stdin);

                } else if (strcmp(argArray[argIndex], ">") == 0 
                        && argIndex < (*numArgs - 1)) {
                    redirSTDOUT = 1;
                    outfile = open(argArray[argIndex + 1], O_CREAT | O_WRONLY | O_TRUNC, 0777);
                    fflush(stdout);
                    if (outfile == -1) {
                        printf("error opening outfile\n");
                        fflush(stdout);
                        exit(1);
                    }

                    dup2(outfile, STDOUT_FILENO);

                    int j = argIndex;
                    while(j < (*numArgs - 2)) {
                       char* temp = argArray[j]; 
                       argArray[j] = argArray[j + 2];
                       argArray[j + 2] = temp;
                       j++;
                    }

                    free(argArray[*numArgs - 1]);
                    argArray[*numArgs - 1] = NULL;
                    free(argArray[*numArgs - 2]);
                    argArray[*numArgs - 2] = NULL;
                    (*numArgs) = *numArgs - 2;
                    argIndex = argIndex - 1;
                }
            }


            if(isBackground && backgroundEnabled){
                // Ignore sigint if process is run in background
                fflush(stdout);
                if(!redirSTDIN){
                    infile = open("/dev/null", O_RDONLY);
                    if (infile == -1) {
                        printf("error opening outfile\n");
                        fflush(stdout);
                        exit(1);
                    }
                    dup2(infile, STDIN_FILENO);
                }
                if(!redirSTDOUT){
                    fflush(stdout);
                    outfile = open("/dev/null", O_WRONLY);
                    if (outfile == -1) {
                        printf("error opening outfile\n");
                        fflush(stdout);
                        exit(1);
                    }
                    fflush(stdout);

                    dup2(outfile, STDOUT_FILENO);

                }
            } else {
                sigaction(SIGINT, &SIGINT_action_child, NULL);
            }
            int j = 0;
            for(j = 0; j<*numArgs; j++) {
                fflush(stdout);
            }

            if (execvp(argArray[0], argArray) == -1) {
                // Exit in an error state if the command fails.
                exit(1);
            }
        default:
            if (!(isBackground && backgroundEnabled)) {
                fflush(stdout);
                if (waitpid(pid, &status, 0) == -1) {
                    printf("\nwaitpid error %s\n", strerror(errno));
                    fflush(stdout);
                }
                if (WIFEXITED(status) != 0) {
                    if (WEXITSTATUS(status) != 0) {
                        printf("erro executing command | exit status: %d\n", WEXITSTATUS(status));
                    }
                    fflush(stdout);
                    *prevExitStatus = WEXITSTATUS(status);
                } else {
                    printf("finished waiting | signal value: %d\n", WTERMSIG(status));
                    fflush(stdout);
                    *prevExitStatus = WTERMSIG(status);
                }
                fflush(stdout);
            } else {
                // Record background proc ids in array.
                int i = 0;
                while (inBackground[i] != 0) {
                    i++;
                }
                (*numInBackground)++;
                inBackground[i] = pid;
                printf("background process started: %d\n", inBackground[i]);
                fflush(stdout);
            }
    }
}

int isComment(char* arg1) {
    if (arg1[0] == '#') {
        return 1; 
    } else {
        return 0;
    }
}

int parseArguments(char* commandString, 
                  int maxArgs, 
                  int* numArgs, 
                  char** argArray,
                  int* prevExitStatus,
                  int* inBackground,
                  int* numInBackground) {
    if (tokenizeArguments(commandString, maxArgs, numArgs, argArray) == -1) {
        perror("error tokenizing argument string\n");
        return -1;
    }

    if (isBuiltIn(argArray[0])) {
        executeBuiltIn(argArray, *numArgs, *prevExitStatus, inBackground, numInBackground);
    } else if (!isComment(argArray[0]))  {
        executeCommand(argArray, numArgs, prevExitStatus, inBackground, numInBackground);
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


void childHandleSIGINT(int signum) {
    char* message = "\nChild SIGINT received\n";
    write(STDOUT_FILENO, message, strlen(message) + 1);
    _exit(signum);
}

void shellHandleSIGTSTP(int signum) {
    char* message;
    if (backgroundEnabled) {
        backgroundEnabled = false;
        message = "\nSIGTSTP received: disabling background processes\n";
    } else {
        backgroundEnabled = true;
        message = "\nSIGTSTP received: re-enabling background processes\n";
    }
    write(STDOUT_FILENO, message, strlen(message) + 1);
}

void expandPID(char* commandString) {
    char buffer[MAX_COMMAND_LENGTH + 3] = {0};
    strcpy(buffer, commandString);
    char* strLoc = strstr(buffer, "$$");
    int pidIndex = (int) (strLoc - buffer);
    pid_t pid = getpid();
    if (strLoc != NULL) {
        buffer[pidIndex] = '%'; 
        buffer[pidIndex + 1] = 'd';
    }
    sprintf(commandString, buffer, pid);
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
        fflush(stdout);
        printf(": ");
        fflush(stdout);
        numRead = getline(&inputBuffer, &bufferSize, stdin);
        if (numRead == -1) {
            clearerr(stdin);
        } else if (numRead > MAX_COMMAND_LENGTH + 1) {
            free(inputBuffer);
            inputBuffer = NULL;
        } else if (numRead > 1){
            break;
        }
    }

    inputBuffer[strcspn(inputBuffer, "\n")] = '\0';
    strcpy(commandString, inputBuffer);
    free(inputBuffer);
    inputBuffer = NULL;
}
