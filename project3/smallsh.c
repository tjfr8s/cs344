#define _POSIX_C_SOURCE 1
#define _GNU_SOURCE
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
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
    size_t  maxCommandStringLength = MAX_COMMAND_LENGTH + 1;
    char    commandString[maxCommandStringLength];
    char*   argArray[MAX_ARGS] = {};
    int     numArgs = 0;

    memset(&commandString, 0, maxCommandStringLength);
    getInputString(commandString);
    printf("The input string: %s\n", commandString);

    tokenizeArguments(commandString, MAX_ARGS, &numArgs, argArray);
    freeArgArray(argArray, numArgs);

    return 0;
}



int parseArguments(char* commandString, 
                  int maxArgs, 
                  int* numArgs, 
                  char** argArray) {
    tokenizeArguments(commandString, maxArgs, numArgs, argArray);

    return 0;
}

void freeArgArray(char** argArray, int numArgs) {
    int curArg = 0;
    for(curArg = 0; curArg < numArgs; curArg++) {
        free(argArray[curArg]);
    }
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
        printf("token: %s\n", token);

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

void getlineHandleSIGINT(int signum) {
    char* message = "SIGINT received\n";
    write(STDOUT_FILENO, message, strlen(message) + 1);
}

/*******************************************************************************
 * Description: This function gets input from the user of no more than
 * MAX_COMMAND_LENGTH characters. It is robust to singnal interruption.
 *
 * @param   char*   commandString   A buffer in which to store the user's 
 *                                  command.
*******************************************************************************/
void getInputString(char* commandString) {
    struct sigaction   SIGINT_action = {{0}};
    SIGINT_action.sa_handler = getlineHandleSIGINT;
    sigfillset(&SIGINT_action.sa_mask);
    sigaction(SIGINT, &SIGINT_action, NULL);

    size_t      bufferSize = 0;
    char*       inputBuffer = NULL;
    int         numRead = 0;


    while(1) {
        printf("Enter a command: ");
        numRead = getline(&inputBuffer, &bufferSize, stdin);
        if (numRead == -1) {
            clearerr(stdin);
        } else if (numRead > MAX_COMMAND_LENGTH + 1) {
            printf("Input exceeded %d characters!\n", MAX_COMMAND_LENGTH);
            free(inputBuffer);
            inputBuffer = NULL;
        } else {
            break;
        }
    }


    inputBuffer[strcspn(inputBuffer, "\n")] = '\0';
    printf("This is what you entered: \"%s\"\n", inputBuffer);
    strcpy(commandString, inputBuffer);
    free(inputBuffer);
    inputBuffer = NULL;
}
