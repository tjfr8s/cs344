/*******************************************************************************
 * Author: Tyler Freitas
 * Date: 190526
 * Description: This program implements a shell through which build-in commands
 * and external programs can be executed. The shell allows for input redirection
 * using the < and > characters. A process can be run in the background using
 * the & character at the end of a command. Background processes can be disabled
 * by sending a SIGTSTP, and re-enabled by sending a SIGTSTP again.
*******************************************************************************/
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

// Static variable indicating whether background commands can be executed.
static bool backgroundEnabled = true;

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

        parseArguments(commandString, 
                MAX_ARGS, 
                &numArgs, 
                argArray, 
                &prevExitStatus, 
                inBackground, 
                &numInBackground);

        freeArgArray(argArray, &numArgs);
    }

    return 0;
}


/*******************************************************************************
 * Description: This function takes an array of process ids of the current
 * background processes, and the number of background processes as arguments
 * and waits for each of these processes.
 *
 * @param   int*    inBackground        The array of process ids of background
 *                                      processes.
 *
 * @param   int*    numInBackground     The number of processes run in the bg.  
*******************************************************************************/
void checkBackgroundProcs(int* inBackground, int* numInBackground) {
    int status = 0;
    int waitVal = 0;
    int i;

    // Loop throug the entire array of processes, wait for all non-zero
    // process ids.
    for (i = 0; i < MAX_BG_PROCS; i++) {
        if (inBackground[i] > 0) {
            if ((waitVal = waitpid(inBackground[i], &status, WNOHANG)) 
                    == -1 && errno != 10) {
                printf("\nwaitpid error %s\n", strerror(errno));
                fflush(stdout);
            } else if (waitVal != 0) {
                (*numInBackground)--;
                if (WIFEXITED(status) != 0) {
                    printf("process id: %d, exit status: %d\n", 
                            inBackground[i], WEXITSTATUS(status));
                    fflush(stdout);
                } else {
                    printf("process id: %d, exit signal: %d\n", 
                            inBackground[i], WTERMSIG(status));
                    fflush(stdout);
                }
                inBackground[i] = 0;
            }
        }
    } 
}


/*******************************************************************************
 * Description: This function checks the first argument of a command and
 * determines if it is a built-in command or not. The function returns true
 * if the command is a built-in and false otherwise.
 *
 * @param   char*   command        The command to check for built-in-ness. 
*******************************************************************************/
bool isBuiltIn(char* command) {
    char*   builtIns[3] = {"exit", "cd", "status"};
    int     i;
    for(i = 0; i < 3; i++) {
        if (!strcmp(command, builtIns[i])) {
            return true;
        }
    }
    return false;
}

/*******************************************************************************
 * Description: This function checks to see if the number of arguments provided
 * with a command is within the correct range of possible arguments. The 
 * function returns true if the command has a valid number of arguments and 
 * false otherwise.
 *
 * @param   char**  argArray        The array of command arguments.
 * @param   int     numArgs         The number of arguments in the command.
 * @param   int     maxArgs         Maixum number of agrs for the command.
 * @param   int     minArgs         Minium number of args for the command.
 * @param   bool    isBuildIn       Flag indicating whether the command is a 
 *                                  built-in or not.
*******************************************************************************/
bool hasValidNumArgs(char** argArray, 
                     int numArgs, 
                     int maxArgs, 
                     int minArgs, 
                     bool isBuiltIn) {
    if (numArgs < minArgs) {
        // Check that command has min args.
        perror("too few arguments\n");
        return false;
    } else if ((numArgs == maxArgs + 1) 
            && isBuiltIn 
            && !strcmp(argArray[numArgs - 1], "&")) {
        // Check if the command has exceeded the maximum number of arguments
        // in the built-in case.
        return true; 
    } else if (numArgs > maxArgs) {
        perror("too many arguments\n");
        return false;
    } else {
        return true;
    }
}


/*******************************************************************************
 * Description: This function executes a build-in command. 
 *
 * @param   char**      argArray        The array of command arguments.
 * @param   int         numArgs         Number of command arguments.
 * @param   int         prevExitStatus  The exit status of prev command.
 * @param   int*        inBackground    Array of background commands
 * @param   int*        numInBackground Number of background commands.
*******************************************************************************/
void executeBuiltIn(char** argArray, 
        int numArgs, 
        int prevExitStatus, 
        int* inBackground, 
        int* numInBackground) {
                 
    // Execute exit command.
    if (!strcmp("exit", argArray[0])
            && hasValidNumArgs(argArray, numArgs, 1, 1, true)) {
        freeArgArray(argArray, &numArgs);

        int i;
        // Kill background processes if there are any running.
        for (i = 0; i < MAX_BG_PROCS; i++) {
            if (inBackground[i] != 0) {
                printf("kill %d\n", inBackground[i]);
                fflush(stdout);
                kill(inBackground[i], SIGKILL);
            }
        }
        exit(0);
    // Change directories.
    } else if (!strcmp("cd", argArray[0])
           && hasValidNumArgs(argArray, numArgs, 3, 1, true)) {
        char* path = NULL;
        if (numArgs == 1) {
            chdir(getenv("HOME"));
        } else {
            path = argArray[1];
            chdir(path);
        }
    // Print the exit status of the last run non-bulit-in command.    
    } else if (!strcmp("status", argArray[0])
           && hasValidNumArgs(argArray, numArgs, 1, 1, true)) {
        printf("status: %d\n", prevExitStatus);
            
        fflush(stdout);
    } 
}

/*******************************************************************************
 * Description: This function creates a child process and executes a command
 * with proper file descriptor redirection and the ability to run the command
 * in the background.
 *
 * @param   char**      argArray        The array of arguments.
 * @param   int*        numArgs         The number of arguments.
 * @param   int*        prevExitStatus  The exit status of the prev command.
 * @param   int*        inBackground    Array of background processes.
 * @param   int*        numInBackground The number of running bg processes.
*******************************************************************************/
void executeCommand(char** argArray, 
    int* numArgs, 
    int* prevExitStatus, 
    int* inBackground, 
    int* numInBackground) {

    int                 status = -5;
    bool                isBackground = false;
    struct sigaction    SIGINT_action_child = {{0}};
    struct sigaction    SIGTSTP_action_child = {{0}};
    int                 redirSTDIN  = 0;
    int                 redirSTDOUT = 0;
    int                 argIndex = 0;
    int                 infile = 0;
    int                 outfile = 0;
    pid_t               pid;

    // Set up signal handlers for SIGINT and SIGTSTP signals.
    SIGINT_action_child.sa_handler = childHandleSIGINT;
    SIGINT_action_child.sa_flags = SA_RESTART | SA_NODEFER;
    sigfillset(&SIGINT_action_child.sa_mask);

    SIGTSTP_action_child.sa_handler = SIG_IGN;
    SIGTSTP_action_child.sa_flags = SA_RESTART | SA_NODEFER;
    sigfillset(&SIGTSTP_action_child.sa_mask);

    // Remove & from background command and set the isBackground flag.
    if (!strcmp(argArray[*numArgs - 1], "&")) {
        free(argArray[*numArgs - 1]);
        argArray[*numArgs - 1] = NULL;
        (*numArgs)--;
        isBackground = true;
    }

    // Fork a child process for executing the command.
    pid = fork();
    switch(pid) {
        case -1:
            // Handle errors in process creation.
            perror("\nerror creating new process\n");
            fflush(stdout);
        case 0:
            // Child process case.
            // Apply the child's SIGSTP signal handler.
            sigaction(SIGTSTP, &SIGTSTP_action_child, NULL);

            // Parse file descriptor redirection.
            for (argIndex = 0; argIndex < *numArgs; argIndex++) {
                // Handle input redirection.
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

                    // Remove the redirection arguments from the command.
                    free(argArray[*numArgs - 1]);
                    argArray[*numArgs - 1] = NULL;
                    free(argArray[*numArgs - 2]);
                    argArray[*numArgs - 2] = NULL;
                    (*numArgs) = *numArgs - 2;
                    argIndex = argIndex - 1;

                } else if (strcmp(argArray[argIndex], ">") == 0 
                        && argIndex < (*numArgs - 1)) {
                    // Handle output redirection.
                    redirSTDOUT = 1;
                    outfile = open(argArray[argIndex + 1], 
                            O_CREAT | O_WRONLY | O_TRUNC, 0777);
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

                    // Remove redirection arguments from the command.
                    free(argArray[*numArgs - 1]);
                    argArray[*numArgs - 1] = NULL;
                    free(argArray[*numArgs - 2]);
                    argArray[*numArgs - 2] = NULL;
                    (*numArgs) = *numArgs - 2;
                    argIndex = argIndex - 1;
                }
            }


            // Handle background process and foreground process specific tasks.
            if(isBackground && backgroundEnabled){
                // Ignore sigint if process is run in background and redirect
                // stdout stdin to dev/null if no redirection has been performed
                // by the user.
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
                    outfile = open("/dev/null", O_WRONLY);
                    if (outfile == -1) {
                        printf("error opening outfile\n");
                        fflush(stdout);
                        exit(1);
                    }
                    dup2(outfile, STDOUT_FILENO);

                }
            } else {
                // Apply SIGINT handler to child process.
                sigaction(SIGINT, &SIGINT_action_child, NULL);
            }


            // Execute the command and exit with status 1, if the command 
            // fails.
            if (execvp(argArray[0], argArray) == -1) {
                // Exit in an error state if the command fails.
                exit(1);
            }

        default:
            // Parent process case.
            // Handle foreground processes (process run without &, or with &
            // while background processes are disabled).
            if (!(isBackground && backgroundEnabled)) {
                if (waitpid(pid, &status, 0) == -1) {
                    printf("\nwaitpid error %s\n", strerror(errno));
                    fflush(stdout);
                }
                if (WIFEXITED(status) != 0) {
                    if (WEXITSTATUS(status) != 0) {
                        printf("error executing command | exit status: %d\n", 
                                WEXITSTATUS(status));
                        fflush(stdout);
                    }
                    *prevExitStatus = WEXITSTATUS(status);
                } else {
                    printf("exited with signal | signal value: %d\n", 
                            WTERMSIG(status));
                    fflush(stdout);
                    *prevExitStatus = WTERMSIG(status);
                }
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

/*******************************************************************************
 * Description: This function returns 1 if a command is a comment and 0 other-
 * wise. 
 *
 * @param   char*   arg1        The first argument of a command. 
*******************************************************************************/
int isComment(char* arg1) {
    if (arg1[0] == '#') {
        return 1; 
    } else {
        return 0;
    }
}

/*******************************************************************************
 * Description: This function breaks a command strig into its component 
 * arguments and performs the proper execution routine based on the command
 * contents. The function returns -1 on error and 0 otherwise.
 *
 * @param   char*   commandString   The command string. 
 * @param   int     maxArgs         Max number of args for command.
 * @param   int*    numArgs         Destination for length of tokenized argArray 
 * @param   char**  argArray        Destination for tokenized arguments.
 * @param   int*    prevExitStatus  Exit status of prev command.
 * @param   int*    inBackground    Array of background process ids. 
 * @param   int*    numInBackground Number of background processes.
*******************************************************************************/
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

    printf("\n");
    fflush(stdout);
    int i;
    for(i = 0; i < *numArgs; i++){
        printf("%s ", argArray[i]);
        fflush(stdout);
    }
    printf("\n");
    fflush(stdout);


    // Execute built-in commands or external programs.
    if (isBuiltIn(argArray[0])) {
        executeBuiltIn(argArray, *numArgs, *prevExitStatus, inBackground, numInBackground);
    } else if (!isComment(argArray[0]))  {
        executeCommand(argArray, numArgs, prevExitStatus, inBackground, numInBackground);
    }


    return 0;
}

/*******************************************************************************
 * Description: This function frees the memory allocated to argArray. 
 *
 * @param   char**      argArray        The array of arguments.
 * @param   int*        numArgs         Number of args in argArray.
*******************************************************************************/
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
    char*    delim = " ";

    // Split arguments at each space character.
    token = strtok(commandString, delim);
    while (token != NULL) {
        if((argArray[*numArgs] = 
                    malloc(sizeof(char) * (strlen(token) + 1))) == NULL) {
            perror("error allocating memory for arg array");
            return -1;
        }
        memset(argArray[*numArgs], 0, strlen(token) + 1);

        // Store argument tokens in argArray.
        strcpy(argArray[*numArgs], token); 
        (*numArgs)++;
        token = strtok(NULL, delim); 
    }

    return 0;
}


/*******************************************************************************
 * Description: This function prints a message and exits with the caught 
 * signal's signum when a signal is caught. 
 *
 * @param   int     signum      Signal number of caught sig.
*******************************************************************************/
void childHandleSIGINT(int signum) {
    char* message = "\nChild SIGINT received\n";
    write(STDOUT_FILENO, message, strlen(message) + 1);
    _exit(signum);
}

/*******************************************************************************
 * Description: This function prints a message and exits with the caught 
 * signal's signum when a signal is caught. It also toggles the ability to 
 * execute background commands in the shell.
 *
 * @param   int     signum      Signal number of caught sig.
*******************************************************************************/
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
    write(STDOUT_FILENO, ": ", strlen(": ") + 1);
}

/*******************************************************************************
 * Description: This function expands $$ to the calling processes pid in the
 * command string.
 *
 * @param   char*   commandString   Command string to be expanded.
*******************************************************************************/
void expandPID(char* commandString) {
    char buffer[MAX_COMMAND_LENGTH + 3] = {0};
    strcpy(buffer, commandString);
    // Get pointer to start of $$;
    char* strLoc = strstr(buffer, "$$");
    // Get index of start of $$
    int pidIndex = (int) (strLoc - buffer);
    pid_t pid = getpid();
    // Replace $$ with string format characters %d
    if (strLoc != NULL) {
        buffer[pidIndex] = '%'; 
        buffer[pidIndex + 1] = 'd';
    }
    memset(commandString, 0, sizeof(char) * (MAX_COMMAND_LENGTH + 5));
    // Print formated string into comandString with pid in place of $$.
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
        // Display input prompt.
        printf(": ");
        fflush(stdout);
        numRead = getline(&inputBuffer, &bufferSize, stdin);
        if (numRead == -1) {
            clearerr(stdin);
        } else if (numRead > MAX_COMMAND_LENGTH + 1) {
            free(inputBuffer);
            printf("\nexceed max\n");
            fflush(stdout);
            inputBuffer = NULL;
        } else if (numRead > 1){
            break;
        }
    }

    inputBuffer[strcspn(inputBuffer, "\n")] = '\0';
    fflush(stdout);
    strcpy(commandString, inputBuffer);
    inputBuffer = NULL;
}
