// REQUIREMENTS:
// Create rooms directory named `freitast.rooms.processid`.

// Create 7 room files in freitast.rooms.processid directory with hard coded
// names.

// room
// -roomName
//  - 8 char max length
//  - hard code 10 options and randomly pick 7
// -roomType
//  - START_ROOM
//  - END_ROOM
//  - MID_ROOM
//  - randomly generate which room gets which type (1 start and 1 end)
// -outboundConnections
//  - each room has 3-6 outbound connections to other rooms
//  - randomly assigned and outbound connections must always return.
//  - file format
//      ROOM NAME: <room name>
//      CONNECTION 1: <room name>
//      ...
//      ROOM TYPE: <room type>
//
//
//
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <stdbool.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>


#define ROOM_GRAPH_SIZE 7

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
    KENNEL
};

enum RoomType {
    START_ROOM,
    END_ROOM,
    MID_ROOM
   
};

struct Room {
    enum RoomName  roomName;
    enum RoomType  roomType;
    struct Room*  outboundConnections[6];
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
    }
}

void room_name_to_string(struct Room* room) {
    switch (room->roomName) {
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
    }
}

void print_connections(struct Room* room) {
    int i;
    for (i = 0; i < room->numConnections; i++) {
        print_room_name(room->outboundConnections[i]);
        printf("\t");
    }
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

char* room_name_to_string() {
    return "lala";
}

// Returns true if all rooms have 3 to 6 outbound connections.
bool is_graph_full(struct Room* roomGraph){
    int i;
    for (i = 0; i < ROOM_GRAPH_SIZE; i++) {
        if (roomGraph[i].numConnections < 3) {
            return false;
        }
    }
    return true;
}

// Returns a random Room, does not validate if a connection can be added
struct Room* get_random_room(struct Room* roomList) {
    int randomRoom = (rand() % (6 - 0 + 1));
    return &roomList[randomRoom];
}

// Returns true if a connection can be added from Rom x (< 6 outbound 
// connections), valse otherwise
bool can_add_connection_from(struct Room* x) {
    if (x->numConnections <= 6) {
        return true;
    } else {
        return false;
    }
}

bool connection_already_exists(struct Room* x, struct Room* y) {
    int i;
    for (i = 0; i < x->numConnections; i++) {
        if (x->outboundConnections[i] == y) {
            return true;
        }
    }
    return false;
}

void connect_room(struct Room* x, struct Room* y) {
    x->outboundConnections[x->numConnections] = y;
    x->numConnections++;
    return;
}

bool is_same_room(struct Room* x, struct Room* y) {
    if (x == y) {
        return true;
    } else {
        return false;
    }
}


// Adds a random connection between a room and another room.
void add_random_connection(struct Room* roomList) {
    struct Room* A;
    struct Room* B;

    while (true) {
        A = get_random_room(roomList);

        if(can_add_connection_from(A) == true
                && A->numConnections < 6) {
            break;
        }

    }

    do {
        B = get_random_room(roomList);
    } while (can_add_connection_from(B) == false 
            || is_same_room(A,B) == true 
            || connection_already_exists(A, B) == true);

    connect_room(A, B);
    connect_room(B, A);

    return;
}

// Choose a random name for each room in the list.
void generate_random_names(struct Room* roomList) {
    bool   usedNames[10];

    int i;
    for (i = 0; i < 10; i++) {
        usedNames[i] = false;
    }

    int  numNames = 0;

    while (numNames < ROOM_GRAPH_SIZE) {
        int randomNameIndex = (rand() % (9 - 0 + 1));

        if (!usedNames[randomNameIndex]) {
            usedNames[randomNameIndex] = true; 
            roomList[numNames].roomName = (enum RoomName)randomNameIndex;
            roomList[numNames].roomType = MID_ROOM;
            roomList[numNames].numConnections = 0;
            numNames++;
        } 
    }
    return;
}

// Choose a room to be the starting room and choose another room to be the
// ending room.
void choose_start_and_end(struct Room* roomList) {
    int randomRoom = (rand() % (6 - 0 + 1));
    roomList[randomRoom].roomType = START_ROOM;

    do {
        randomRoom = (rand() % (6 - 0 + 1));
    } while (roomList[randomRoom].roomType == START_ROOM);
    roomList[randomRoom].roomType = END_ROOM;
}


// Initialze the roomName and roomType of each room.
void initialize_room_list(struct Room* roomList) {
    generate_random_names(roomList);
    choose_start_and_end(roomList);
    int i;
    while (!is_graph_full(roomList)) {
        for (i = 0; i < ROOM_GRAPH_SIZE; i++) {
            print_room(&roomList[i]);
        }
        add_random_connection(roomList);    
    }
    return;
}

void write_room_to_file(FILE* ifp, struct Room* room) {
    fputs("ROOM NAME: %s", room_name_to_string(room->roomName));
}


int main(const int argc, char** argv) {
    char dir_name[50];
    sprintf(dir_name, "./freitast.rooms.%d", getpid());
    mkdir(dir_name, S_IRWXU);


    srand(time(NULL));
    struct Room roomList[ROOM_GRAPH_SIZE];
    int i;
    
    initialize_room_list(roomList); 

    char file_path[50];
    int i;
    for (i = 0; i < 7; i++) {
        sprintf(file_path, "%s/room%d", dir_name, i);
        FILE* fp = fopen(file_path, "w");
        fclose(fp);
    }

    for (i = 0; i < ROOM_GRAPH_SIZE; i++) {
        print_room(&roomList[i]);
    }
    return 0;
}

