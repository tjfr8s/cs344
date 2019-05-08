/*******************************************************************************
 * Author: Tyler Freitas
 * Date: 05/08/2019
 * Description: This program implements a dungeon crawling game. The player
 * is placed in a room and provided with rooms to move into until they find the
 * exit. The user can get the current time by typing the word `time` when
 * prompted for a room.
*******************************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <dirent.h> 
#include <unistd.h>
#include <string.h>
#include <fcntl.h>
#include <limits.h>
#include <pthread.h>
#include <signal.h>
#define BUF_SIZE 1024
#define ROOM_GRAPH_SIZE 7
#define MAX_CONNECTIONS 6
#define STRING_BUF_SIZE 256
#define TIME_FILE_PATH "./currentTime.txt"

enum RoomName {
    MAINHALL,
    DARKROOM,
    BATHROOM,
    STARS,
    ARTROOM,
    LOUNGE,
    ARMORY,
    ROOF,
    MORGUE,
    KENNEL,
    NONE_NAME
};

enum RoomType {
    START_ROOM,
    END_ROOM,
    MID_ROOM,
    NONE_TYPE
   
};

enum bool {
    false,
    true
};

struct Room {
    enum RoomName  roomName;
    enum RoomType  roomType;
    struct Room*   outboundConnections[MAX_CONNECTIONS];
    int            numConnections;
};

void print_room_name(struct Room* room);
char* room_name_to_string(enum RoomName roomName);
enum RoomName string_to_room_name(char* name_string);
char* room_type_to_string(enum RoomType roomType);
enum RoomType string_to_room_type(char* type_string);
void print_connections(struct Room* room);
void print_room(struct Room* room);
void get_user_choice();
void get_room_dir(char* newestDirName);
void file_to_room_connection(struct Room* rooms[], 
        struct Room* curRoom,
        enum RoomName nameOfConnection);
int load_rooms(struct Room* rooms[]);
void initialize_game_state(struct Room** playerLoc, struct Room* rooms[]);
void display_prompt(struct Room* playerLoc, struct Room* rooms[]);
void get_time();
enum bool move_rooms(struct Room** playerLoc, struct Room* rooms[]);
void add_to_history(char*** history, 
                    int* historyIndex, 
                    int* historySize, 
                    enum RoomName roomName);
void* time_thread_init(void* initParam);
void print_time_from_file();


// Initialize mutex and condition for managing thread access to currentTime.txt
pthread_cond_t cond1 = PTHREAD_COND_INITIALIZER;
pthread_mutex_t mutex1 = PTHREAD_MUTEX_INITIALIZER;

int main(const int argc, char** argv) {
    // Lock the mutex in the main thread to give this thread since the time 
    // keeping thread won't need it until its writing the time to a file.
    pthread_mutex_lock(&mutex1);

    struct Room* rooms[ROOM_GRAPH_SIZE];
    struct Room* playerLoc = NULL;
    int          i;
    int          j;
    int          historySize = 2;
    int          historyIndex = 0;
    char**       history = malloc(sizeof(char*) * historySize);
    pthread_t    timeThread;

    // Create the thread that will write the current time to a file.
    pthread_create(&timeThread, NULL, time_thread_init, NULL);

    // Initialize array used to store movement history.
    memset(history, 0, sizeof(char*) * historySize);
    for (i = 0; i < historySize; i++) {
        history[i] = malloc(sizeof(char) * 10);
        memset(history[i], 0, sizeof(char) * 10);
    }

    // Initialize each room struct in the array of rooms.
    for (i = 0; i < ROOM_GRAPH_SIZE; i++) {
        rooms[i] = (struct Room*) malloc(sizeof(struct Room));
        rooms[i]->roomName = NONE_NAME;
        rooms[i]->numConnections = 0;
        rooms[i]->roomType = MID_ROOM;
        for (j = 0; j < MAX_CONNECTIONS; j++) {
            rooms[i]->outboundConnections[j] = NULL;
        }
    }

    initialize_game_state(&playerLoc, rooms);

    enum bool hasQuit = false;
    enum bool hasWon = false;
    enum bool moveStatus = true;

    // Execute the game loop until the user has won.
    while (!hasQuit && !hasWon) {
        // Record movements in the history.
        if (moveStatus == true) {
            add_to_history(&history, &historyIndex, 
                           &historySize, playerLoc->roomName);
        }
        do {
            if (playerLoc->roomType == END_ROOM) {
                hasWon = true; 
            } else {
                display_prompt(playerLoc, rooms);
            }
        } while(!hasWon 
                && (moveStatus = move_rooms(&playerLoc, rooms)) == false);
    }

    // Display the user's history and the victory message.
    printf("YOU HAVE FOUND THE END ROOM. CONGRATULATIONS! YOU TOOK %d STEPS.\n" \
            "YOUR PATH TO VICTORY WAS:\n", historyIndex);
    for (i = 0; i < historyIndex; i++) {
        printf("%s\n", history[i]);
    }

    for (i = 0; i < ROOM_GRAPH_SIZE; i++) {
        free(rooms[i]);
    }
    for(i = 0; i < historySize; i++) {
        free(history[i]);
    }
    free(history);

    pthread_cancel(timeThread);



    return 0;
}

/*******************************************************************************
 * Description: This function takes a struct Room and prints its name to the
 * console.
 * @param   struct Room*    room    the room that will have its name printed.
*******************************************************************************/
void print_room_name(struct Room* room) {
    switch (room->roomName) {
        case MAINHALL: 
            printf("MAINHALL");
            break;
        case DARKROOM:
            printf("DARKROOM");
            break;
        case BATHROOM:
            printf("BATHROOM");
            break;
        case STARS:
            printf("STARS");
            break;
        case ARTROOM:
            printf("ARTROOM");
            break;
        case LOUNGE:
            printf("LOUNGE");
            break;
        case ARMORY:
            printf("ARMORY");
            break;
        case ROOF:
            printf("ROOF");
            break;
        case MORGUE:
            printf("MORGUE");
            break;
        case KENNEL:
            printf("KENNEL");
            break;
        case NONE_NAME:
            printf("NONE_NAME");
            break;
    }
}


/*******************************************************************************
 * Description: This function takes a struct Room and returns its name as a 
 * string.
 * @param   struct Room*    room    the room that will have its name returned.
 *
 * @return  char*                   the room's name as a string.
*******************************************************************************/
char* room_name_to_string(enum RoomName roomName) {
    switch (roomName) {
        case MAINHALL: 
            return "MAINHALL";
            break;
        case DARKROOM:
            return "DARKROOM";
            break;
        case BATHROOM:
            return "BATHROOM";
            break;
        case STARS:
            return "STARS";
            break;
        case ARTROOM:
            return "ARTROOM";
            break;
        case LOUNGE:
            return "LOUNGE";
            break;
        case ARMORY:
            return "ARMORY";
            break;
        case ROOF:
            return "ROOF";
            break;
        case MORGUE:
            return "MORGUE";
            break;
        case KENNEL:
            return "KENNEL";
            break;
        case NONE_NAME:
            return "NONE_NAME";
            break;
    }
    return "";
}

/*******************************************************************************
 * Description: This function takes a string and returns the appropriate room
 * name (NONE_NAME otherwise).
 * @param   char*           name_string    the room that will have its name 
 *                                         returned.
 *
 * @return  enum RoomName                  the room's name as an enum RoomName.
*******************************************************************************/
enum RoomName string_to_room_name(char* name_string) {
    if (!strcmp(name_string, "MAINHALL")) {
        return MAINHALL;
    } else if (!strcmp(name_string, "DARKROOM")){
        return DARKROOM;
    } else if (!strcmp(name_string, "BATHROOM")){
        return BATHROOM;
    } else if (!strcmp(name_string, "STARS")){
        return STARS;
    } else if (!strcmp(name_string, "ARTROOM")){
        return ARTROOM;
    } else if (!strcmp(name_string, "LOUNGE")){
        return LOUNGE;
    } else if (!strcmp(name_string, "ARMORY")){
        return ARMORY;
    } else if (!strcmp(name_string, "ROOF")){
        return ROOF;
    } else if (!strcmp(name_string, "MORGUE")){
        return MORGUE;
    } else if (!strcmp(name_string, "KENNEL")){
        return KENNEL;
    } else {
        return NONE_NAME;
    }
}

/*******************************************************************************
 * Description: This function maps an enum RoomType to a string.
 *
 * @param   enum RoomType   roomType        room type that will have its enum 
 *                                          value returned.
 *
 * @return  enum RoomName                   the room's type as an enum RoomType.
*******************************************************************************/
char* room_type_to_string(enum RoomType roomType) {
    switch (roomType) {
        case START_ROOM: 
            return "START_ROOM";
            break;
        case END_ROOM:
            return "END_ROOM";
            break;
        case MID_ROOM:
            return "MID_ROOM";
            break;
        case NONE_TYPE:
            return "NONE";
            break;
    }
    return "";
}

/*******************************************************************************
 * Description: This function takes a string and returns the appropriate room
 * type (NONE_TYPE otherwise).
 * @param   char*           name_string    the room that will have its type 
 *                                         returned.
 *
 * @return  enum RoomType                  the room's type as an enum RoomType.
*******************************************************************************/
enum RoomType string_to_room_type(char* type_string) {
    if (!strcmp(type_string, "MID_ROOM")) {
        return MID_ROOM;
    } else if (!strcmp(type_string, "END_ROOM")){
        return END_ROOM;
    } else if (!strcmp(type_string, "START_ROOM")){
        return START_ROOM;
    } else {
        return NONE_TYPE;
    }
}

/*******************************************************************************
 * Description: Print each connection made to this room.
 *
 * @param   struct Room*    room    room to print connections for.
*******************************************************************************/
void print_connections(struct Room* room) {
    int i;
    for (i = 0; i < room->numConnections - 1; i++) {
        print_room_name(room->outboundConnections[i]);
        printf(", ");
    }
    print_room_name(room->outboundConnections[i]);
    printf(".\n");
}

/*******************************************************************************
 * Description: Print the properties of the passed room.
 *
 * @param   struct Room*    room    room to print.
*******************************************************************************/
void print_room(struct Room* room) {
    printf("room name: %d ", room->roomName);
    print_room_name(room);
    printf("\nroom type: %d\n", room->roomType);
    printf("num_connections: %d\n", room->numConnections);
    print_connections(room);
    printf("\n");
    printf("\n");
    return;
}

/*******************************************************************************
 * Description: Finds the name of the newest room directory and passes it out
 * of the function via the newestDirName parameter.
 *
 * @param   char*    newestDirName      Parameter in which to store the newest
 *                                      room directory's name.
*******************************************************************************/
void get_room_dir(char* newestDirName) {
    long            newest_dir_time = -1;
    char            target_dir_prefix[32] = "freitast.rooms.";
    DIR*            to_check; 
    struct dirent*  cur_file;
    struct stat     subdir_attributes;

    memset(newestDirName, '\0', sizeof(char) * STRING_BUF_SIZE);
    to_check = opendir(".");

    // Read through each directory in the current directory while keeping
    // track of the directory with the most recent st_mtime.
    if (to_check > 0) {
        while ((cur_file = readdir(to_check)) != NULL) {
            // Find files that match the room dir prefix.
            if (strstr(cur_file->d_name, target_dir_prefix) != NULL) {
                stat(cur_file->d_name, &subdir_attributes);
                // Check the st_mtime of the current file to see if it could
                // be the newest directory.
                if ((int)subdir_attributes.st_mtime > newest_dir_time) {
                    newest_dir_time = (int)subdir_attributes.st_mtime;
                    memset(newestDirName, 0, sizeof(char) * STRING_BUF_SIZE);
                    strcpy(newestDirName, cur_file->d_name);
                }
            }
        }
    }
    closedir(to_check);
    return;
}

/*******************************************************************************
 * Description: Add address of connected room to outoundConnections of room
 * struct.
 *
 * @param   struct Room*    rooms[]             Array of all rooms. 
 * @param   struct Room*    curRoom             Room to which a conneciton will 
 *                                              be added. 
 * @param   enum RoomName   nameOfConnection    Name of room to connect. 
*******************************************************************************/
void file_to_room_connection(struct Room* rooms[], 
        struct Room* curRoom,
        enum RoomName nameOfConnection) {
    int i;
    // Locate the address of the room associated with nameOfConnection and
    // add that address to curRoom's outboundConnections.
    for (i = 0; i < ROOM_GRAPH_SIZE; i++) {
        if (rooms[i]->roomName == nameOfConnection) {
            curRoom->outboundConnections[curRoom->numConnections] = rooms[i];
            curRoom->numConnections++;
            return;
        }
    }
}

/*******************************************************************************
 * Description: Load the rooms from the newest room directory into the rooms
 * array.
 *
 * @param   struct Room*    rooms[]     target array of rooms to be filled.
*******************************************************************************/
int load_rooms(struct Room* rooms[]) {
    char    roomDirName[STRING_BUF_SIZE];
    struct  dirent* curFile;
    DIR*    curDir;
    char    roomPath[STRING_BUF_SIZE];
    FILE*   ifp;
    int     curRoom = 0;
    ssize_t numRead;
    char*   line = NULL;
    size_t  len = 0;
    char*   token = NULL;

    // Get the name of the newest dir and open it.
    get_room_dir(roomDirName);
    curDir = opendir(roomDirName);

    // extract the name from each room file and insert it into the room array.
    while ((curFile = readdir(curDir)) != NULL) {
        // Ignore '.' and '..' files and read in room names.
        if (strcmp(curFile->d_name, ".") && strcmp(curFile->d_name, "..")) {
            sprintf(roomPath, "%s/%s", roomDirName, curFile->d_name);
            if ((ifp = fopen(roomPath, "r")) == NULL) {
                printf("error");
            }

            // Tokenize the first line of each file to extract name.
            numRead = getline(&line, &len, ifp);  
            token = strtok(line, " ");
            int i;
            for(i = 0; i < 2; i++) {
                token = strtok(NULL, " \n");
            }

            // Update name in room array.
            rooms[curRoom]->roomName = string_to_room_name(token);

            fclose(ifp);
            curRoom++;
        } 
    }


    // Repeat above process, but add connections this time. This needed to
    // be done in two steps becuase the rooms are stored in outboundConnections
    // by address. Thus, room names must be associated with each room before
    // their addresses can be mapped to connections.
    closedir(curDir);
    curDir = opendir(roomDirName);
    curRoom = 0;

    while ((curFile = readdir(curDir)) != NULL) {
        if (strcmp(curFile->d_name, ".") && strcmp(curFile->d_name, "..")) {
            sprintf(roomPath, "%s/%s", roomDirName, curFile->d_name);
            if ((ifp = fopen(roomPath, "r")) == NULL) {
                printf("error");
            }

            // Skip the first line (name line) in the file and read in all 
            // other lines.
            int j = 0;
            getline(&line, &len, ifp);
            while((numRead = getline(&line, &len, ifp)) != -1) {
                // Tokenize line to determine if it is a connection or a type.
                token = strtok(line, " ");
                if (!strcmp("CONNECTION", token)) {
                    // Add connection to outboundConnections array.
                    token = strtok(NULL, " \n");
                    token = strtok(NULL, " \n");
                    file_to_room_connection(rooms, 
                            rooms[curRoom], 
                            string_to_room_name(token));
                } else if (!strcmp("ROOM", token)) {
                    // Add room type.
                    token = strtok(NULL, " \n");
                    token = strtok(NULL, " \n");
                    if (!strcmp("START_ROOM", token)) {
                        rooms[curRoom]->roomType = START_ROOM;

                    } else if (!strcmp("END_ROOM", token)) {
                        rooms[curRoom]->roomType = END_ROOM;
                    }
                }
                j++;
            }

            fclose(ifp);
            curRoom++;
        } 
    }
    closedir(curDir);
    free(line);

    return 0;        
}

/*******************************************************************************
 * Description: Initializes the room and playerLoc structs.
 *
 * @param   struct Room*    rooms[]     target array of rooms to be initialized.
 * @param   struct Room*    playerLoc   room that the player starts in.
 * 
*******************************************************************************/
void initialize_game_state(struct Room** playerLoc, struct Room* rooms[]) {
    load_rooms(rooms);
    enum bool startFound = false; 
    int i = 0;
    // Find the start room and point playerLoc to it.
    while (i < ROOM_GRAPH_SIZE && !startFound) {
        if (rooms[i]->roomType == START_ROOM) {
            *playerLoc = rooms[i];
            startFound = true;
        }
        i++;
    }
}

/*******************************************************************************
 * Description: Display the prompt at before each movement.
 *
 * @param   struct Room*    rooms[]     array of rooms.
 * @param   struct Room*    playerLoc   room that the player is in.
 * 
*******************************************************************************/
void display_prompt(struct Room* playerLoc, struct Room* rooms[]) {
    printf("CURRENT LOCATION: %s\n", room_name_to_string(playerLoc->roomName));
    printf("POSSIBLE CONNECTIONS: ");
    print_connections(playerLoc);
    printf("WHERE TO? >");
}

/*******************************************************************************
 * Description: Gets the current time and writes it to a file called 
 * currentTime.txt.
*******************************************************************************/
void get_time() {
    time_t t;
    struct tm* curTime;
    char timeBuf[200];

    // Get local time.
    time(&t);
    curTime = localtime(&t);

    if (curTime == NULL) {
        perror("localtime");
        exit(EXIT_FAILURE);
    }

    // Format the time and write it to a file.
    strftime(timeBuf, 200, "%I:%M%P, %A, %B %d, %Y", curTime);
    FILE* ofp = fopen(TIME_FILE_PATH, "w");
    fprintf(ofp, "%s\n", timeBuf);
    fclose(ofp);
}

/*******************************************************************************
 * Description: Opens currentTime.txt and displays the time stored within.
*******************************************************************************/
void print_time_from_file() {
    // Wait for the other thread to signal the condition. Then acquire the 
    // mutex.
    pthread_cond_wait(&cond1, &mutex1);

    // Print the time from the currentTime.txt file.
    FILE* ifp = fopen(TIME_FILE_PATH, "r");
    if (ifp == NULL) {
    } else {
        char timeBuf[200];
        fgets(timeBuf, 200, ifp);
        printf("\n%s", timeBuf);
        fclose(ifp);
    }
}

/*******************************************************************************
 * Description: This function orchestrates the player's movement between rooms.
 *
 * @param   struct Room*    rooms[]     array of rooms.
 * @param   struct Room*    playerLoc   room that the player is in.
 * 
 * @return  enum bool                   returns true if the player moved and
 *                                      false if not.
*******************************************************************************/
enum bool move_rooms(struct Room** playerLoc, struct Room* rooms[]) {
    enum bool       successfulMove = true;
    char            choiceBuf[STRING_BUF_SIZE];
    char*           choice;
    enum RoomName   roomNameChoice = NONE_NAME;

    // Get the user's room choice.
    memset(&choiceBuf, 0, sizeof(char) * STRING_BUF_SIZE);
    if (!fgets(choiceBuf, 10, stdin)) {
        printf("error getting user input");
        successfulMove = false;
    } else if (!strcmp("\n", choiceBuf)) {
        // Handle the case where the user hits enter without providing input.
        successfulMove = false;
    }

    // Make sure the user entered a room successfully and that they didn't 
    // request the time.
    if (successfulMove == true && strcmp(choiceBuf, "time\n")) {
        choice = strtok(choiceBuf, "\n");

        roomNameChoice = string_to_room_name(choice);
        enum bool isValid = false;
        int i;
        for (i = 0; i < (*playerLoc)->numConnections && !isValid; i++) {
            if (roomNameChoice == (*playerLoc)->outboundConnections[i]->roomName) {
                isValid = true;
                *playerLoc = (*playerLoc)->outboundConnections[i];
            }
        }
        if (!isValid) {
            successfulMove = false;
        }
    } else {
        // If the user requested the time, signal the time thread to wake and
        // release the mutex so that the time thread can access currentTime.txt
        pthread_cond_signal(&cond1);
        pthread_mutex_unlock(&mutex1);
        successfulMove = false;
        print_time_from_file();
    }

    // If the user entered an invalid command (not an accessible room name or
    // the word `time`, then tell them there was an issue.
    if (!successfulMove && strcmp(choiceBuf, "time\n")) {
        printf("\nHUH? I DON’T UNDERSTAND THAT ROOM. TRY AGAIN.\n\n");
    } else {
        printf("\n");
    }


    return successfulMove;
}

/*******************************************************************************
 * Description: Adds a room name to the user's history.
 *
 * @param   char***       history             the array of rooms the user has 
 *                                            visited
 * @param   int*          historyIndex        index of the current history item 
 * @param   int*          historySize         size of history array
 * @param   enum RoomName                     name to add to history
*******************************************************************************/
void add_to_history(char*** history, 
                    int* historyIndex, 
                    int* historySize, 
                    enum RoomName roomName){

    // Reallocate history array when capacity is reached.
    if (*historyIndex == *historySize) {
        int newHistorySize = 2 * *historySize;
        char** newHistory = malloc(sizeof(char*) * newHistorySize);
        int i;
        for (i = 0; i < newHistorySize; i++) {
            newHistory[i] = malloc(sizeof(char) * 10);
            // Move history values from old array.
            if (i < *historySize) {
                strcpy(newHistory[i], (*history)[i]);
                free((*history)[i]);
            }
        }

        // free the old array and replace it with the new one.
        free(*history);
        *history = newHistory;
        *historySize = newHistorySize;
    }

    // add the new history item.
    strcpy((*history)[*historyIndex], room_name_to_string(roomName));
    *historyIndex = *historyIndex + 1;
}

/*******************************************************************************
 * Description: Runs on creation of the time thread and orchestrates the calling
 * of get_time()
 *
 * @param   void*   initParam       empty parameter required for thread 
 *                                  entrypoint.
*******************************************************************************/
void* time_thread_init(void* initParam) {
    while (true) {
        // Wait until the condition is signaled by the other thread, then
        // acquire the mutex.
        pthread_cond_wait(&cond1, &mutex1); 
        // Get the time and write it to file.
        get_time();
        // Signal the other thread that the file is written too and release the
        // mutex.
        pthread_cond_signal(&cond1);
        pthread_mutex_unlock(&mutex1);
    }
    return NULL;
}
