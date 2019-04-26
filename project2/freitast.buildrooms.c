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

char* room_name_to_string() {
    return "lala";
}

// Returns true if all rooms have 3 to 6 outbound connections.
bool is_graph_full(struct Room* roomGraph){
    int i;
    for (i = 0; i < ROOM_GRAPH_SIZE; i++) {
        if (roomGraph[i].numConnections >= 3
                && roomGraph[i].numConnections <=6) {
            return true;
        }
    }
    return false;
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
    y->outboundConnections[y->numConnections] = x;
    x->numConnections++;
    y->numConnections++;
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

        if(can_add_connection_from(A) == true) {
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
        printf("num named %d\n", numNames);
        int randomNameIndex = (rand() % (9 - 0 + 1));
        printf("random name index %d\t", randomNameIndex);
        printf("usedNames %d\n", usedNames[randomNameIndex]);

        if (!usedNames[randomNameIndex]) {
            usedNames[randomNameIndex] = true; 
            roomList[numNames].roomName = (enum RoomName)randomNameIndex;
            roomList[numNames].roomType = MID_ROOM;
            roomList[numNames].numConnections = 0;
            printf("***chosen number %d\n", randomNameIndex);
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
    printf("foo\n");
    generate_random_names(roomList);
    printf("foo\n");
    choose_start_and_end(roomList);
    return;
}



int main(const int argc, char** argv) {
    srand(time(NULL));
    struct Room roomList[ROOM_GRAPH_SIZE];
    int i;
    for (i = 0; i < ROOM_GRAPH_SIZE; i++) {
        printf("room name: %d\t", roomList[i].roomName);
        printf("room type: %d\n", roomList[i].roomType);
    }
    
    initialize_room_list(roomList); 

    for (i = 0; i < ROOM_GRAPH_SIZE; i++) {
        printf("room name: %d\t", roomList[i].roomName);
        printf("room type: %d\n", roomList[i].roomType);
    }
    return 0;
}

