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
#define BUF_SIZE 1024
#define ROOM_GRAPH_SIZE 7
#define MAX_CONNECTIONS 6
#define STRING_BUF_SIZE 256

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

void print_connections(struct Room* room) {
    int i;
    for (i = 0; i < room->numConnections - 1; i++) {
        print_room_name(room->outboundConnections[i]);
        printf(", ");
    }
    print_room_name(room->outboundConnections[i]);
    printf(".\n");
}

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

// Gets a room name to travel to from the user. If the name entered is not a 
// room name, the user is promted again.
void get_user_choice() {
    return;

}


void get_room_dir(char* newestDirName) {
    long    newest_dir_time = -1;
    char    target_dir_prefix[32] = "freitast.rooms.";

    memset(newestDirName, '\0', sizeof(char) * STRING_BUF_SIZE);

    DIR*            to_check; 
    struct dirent*  cur_file;
    struct stat     subdir_attributes;

    to_check = opendir(".");

    if (to_check > 0) {
        while ((cur_file = readdir(to_check)) != NULL) {
            if (strstr(cur_file->d_name, target_dir_prefix) != NULL) {
                stat(cur_file->d_name, &subdir_attributes);
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

// Adds a new connection to the current room by searching for the address
// of the room with the specified name.
void file_to_room_connection(struct Room* rooms[], 
        struct Room* curRoom,
        enum RoomName nameOfConnection) {
    int i;
    for (i = 0; i < ROOM_GRAPH_SIZE; i++) {
        if (rooms[i]->roomName == nameOfConnection) {
            curRoom->outboundConnections[curRoom->numConnections] = rooms[i];
            curRoom->numConnections++;
            return;
        }
    }
}


// Find the newest rooms directory and load its rooms into the program. Returns
// the index of the start room.
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

    get_room_dir(roomDirName);
    curDir = opendir(roomDirName);

    while ((curFile = readdir(curDir)) != NULL) {
        if (strcmp(curFile->d_name, ".") && strcmp(curFile->d_name, "..")) {
            sprintf(roomPath, "%s/%s", roomDirName, curFile->d_name);
            if ((ifp = fopen(roomPath, "r")) == NULL) {
                printf("error");
            }

            numRead = getline(&line, &len, ifp);  
            token = strtok(line, " ");
            int i;
            for(i = 0; i < 2; i++) {
                token = strtok(NULL, " \n");
            }

            rooms[curRoom]->roomName = string_to_room_name(token);

            fclose(ifp);
            curRoom++;
        } 
    }


    closedir(curDir);
    curDir = opendir(roomDirName);
    curRoom = 0;

    while ((curFile = readdir(curDir)) != NULL) {
        if (strcmp(curFile->d_name, ".") && strcmp(curFile->d_name, "..")) {
            sprintf(roomPath, "%s/%s", roomDirName, curFile->d_name);
            if ((ifp = fopen(roomPath, "r")) == NULL) {
                printf("error");
            }

            int j = 0;
            getline(&line, &len, ifp);
            while((numRead = getline(&line, &len, ifp)) != -1) {
                token = strtok(line, " ");
                if (!strcmp("CONNECTION", token)) {
                    // Add connection
                    token = strtok(NULL, " \n");
                    token = strtok(NULL, " \n");
                    file_to_room_connection(rooms, 
                            rooms[curRoom], 
                            string_to_room_name(token));
                } else if (!strcmp("ROOM", token)) {
                    // Add room type
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

// Calls load_rooms and initializes the player's location to be the start room. 
void initialize_game_state(struct Room** playerLoc, struct Room* rooms[]) {
    load_rooms(rooms);
    enum bool startFound = false; 
    int i = 0;
    while (i < ROOM_GRAPH_SIZE && !startFound) {
        if (rooms[i]->roomType == START_ROOM) {
            *playerLoc = rooms[i];
            startFound = true;
        }
        i++;
    }
}

void display_prompt(struct Room* playerLoc, struct Room* rooms[]) {
    printf("CURRENT LOCATION: %s\n", room_name_to_string(playerLoc->roomName));
    printf("POSSIBLE CONNECTIONS: ");
    print_connections(playerLoc);
    printf("WHERE TO? >");
}

enum bool move_rooms(struct Room** playerLoc, struct Room* rooms[]) {
    enum bool successfulMove = true;
    char choiceBuf[STRING_BUF_SIZE];
    char* choice;
    enum RoomName roomNameChoice = NONE_NAME;
    memset(&choiceBuf, 0, sizeof(char) * STRING_BUF_SIZE);
    if (!fgets(choiceBuf, 10, stdin)) {
        printf("error getting user input");
        successfulMove = false;
    } else if (!strcmp("\n", choiceBuf)) {
        // Handle the case where the user hits enter without providing input.
        successfulMove = false;
    }

    if (successfulMove == true) {
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
    }

    if (!successfulMove) {
        printf("\nHUH? I DON’T UNDERSTAND THAT ROOM. TRY AGAIN.\n\n");
    }

    return successfulMove;
}


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

        free(*history);
        *history = newHistory;
        *historySize = newHistorySize;
    }

    strcpy((*history)[*historyIndex], room_name_to_string(roomName));
    *historyIndex = *historyIndex + 1;

}


int main(const int argc, char** argv) {

    struct Room* rooms[ROOM_GRAPH_SIZE];
    struct Room* playerLoc = NULL;
    int i;
    int j;
    int historySize = 2;
    int historyIndex = 0;
    char** history = malloc(sizeof(char*) * historySize);
    memset(history, 0, sizeof(char*) * historySize);

    for (i = 0; i < historySize; i++) {
        history[i] = malloc(sizeof(char) * 10);
        memset(history[i], 0, sizeof(char) * 10);
    }
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
    while (!hasQuit && !hasWon) {
        // display prompt 
        // get user choice
        // update the player's location and record the user's choice in history

        add_to_history(&history, &historyIndex, &historySize, playerLoc->roomName);
        do {
            if (playerLoc->roomType == END_ROOM) {
                hasWon = true; 
            } else {
                display_prompt(playerLoc, rooms);
            }
        } while(!hasWon && move_rooms(&playerLoc, rooms) == false);
    }

    /*
    printf("Room states: \n");
    for (i = 0; i < ROOM_GRAPH_SIZE; i++) {
        print_room(rooms[i]);
    }

    printf("Start room: \n");
    print_room(playerLoc);

    */
    printf("\nYOU HAVE FOUND THE END ROOM. CONGRATULATIONS! YOU TOOK %d STEPS.\n" \
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


    return 0;
}
