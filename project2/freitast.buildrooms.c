/*******************************************************************************
 * Author: Tyler Freitas
 * Date: 05/08/2019
 * Description: This program creates a set of 7 rooms to be used with the 
 * adventure game. It saves them in separate text files in a directory named
 * after the process id of the running program.
*******************************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
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
    KENNEL,
    NONE
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
    struct Room*  outboundConnections[6];
    int            numConnections;
};


void print_room_name(struct Room* room);
char* room_name_to_string(enum RoomName roomName);
char* room_type_to_string(enum RoomType roomType);
void print_connections(struct Room* room);
void print_room(struct Room* room);
enum bool is_graph_full(struct Room* roomGraph);
struct Room* get_random_room(struct Room* roomList);
enum bool can_add_connection_from(struct Room* x);
enum bool connection_already_exists(struct Room* x, struct Room* y);
void connect_room(struct Room* x, struct Room* y);
enum bool is_same_room(struct Room* x, struct Room* y);
void add_random_connection(struct Room* roomList);
void generate_random_names(struct Room* roomList);
void choose_start_and_end(struct Room* roomList);
void initialize_room_list(struct Room* roomList);
void write_room_to_file(FILE* ifp, struct Room* room);

int main(const int argc, char** argv) {
    char dir_name[50];
    // create a directory to save the room files in. The dir name is made
    // appending the current process id to the prefix below.
    sprintf(dir_name, "./freitast.rooms.%d", getpid());
    mkdir(dir_name, S_IRWXU);

    srand(time(NULL));
    struct Room roomList[ROOM_GRAPH_SIZE];
    
    initialize_room_list(roomList); 

    // Write each room in the room array to a file in the room directory.
    int     i;
    char    file_path[50];
    for (i = 0; i < 7; i++) {
        sprintf(file_path, "%s/room%d", dir_name, i);
        FILE* fp = fopen(file_path, "w");
        write_room_to_file(fp, &roomList[i]);
        fclose(fp);
    }

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
        case NONE:
            printf("NONE");
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
        case NONE:
            return "NONE";
            break;
    }
    return "";
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
 * Description: Print each connection made to this room.
 *
 * @param   struct Room*    room    room to print connections for.
*******************************************************************************/
void print_connections(struct Room* room) {
    int i;
    // loop through the rooms connections and print them.
    for (i = 0; i < room->numConnections; i++) {
        print_room_name(room->outboundConnections[i]);
        printf("\t");
    }
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
 * Description: Returns true if each room in the graph has more than 3 
 * connections, and false otherwise.
 *
 * @param   struct Room*    roomGraph   graph to be checked for fullness.
 *
 * @return  enum bool                   true if full, false otherwise.
*******************************************************************************/
enum bool is_graph_full(struct Room* roomGraph) {
    int i;
    // Check if each room has more than three connections.
    for (i = 0; i < ROOM_GRAPH_SIZE; i++) {
        if (roomGraph[i].numConnections < 3) {
            return false;
        }
    }
    return true;
}

/*******************************************************************************
 * Description: returns a random Room, does not validate if a connection can 
 * be added.
 *
 * @param   struct Room*    roomList    list of rooms to chose from.
 *
 * @return  struct Room*                the chosen random room.
*******************************************************************************/
struct Room* get_random_room(struct Room* roomList) {
    // Chosse a random room index and return the room.
    int randomRoom = (rand() % (6 - 0 + 1));
    return &roomList[randomRoom];
}

/*******************************************************************************
 * Description: Returns true if a connection can be added from Rom x (< 6 
 * outbound connections), valse otherwise
 *
 * @param   struct Room*    x   room to check for connectability
 *
 * @return  enum bool           true if the room has less than 6 connections
*******************************************************************************/
enum bool can_add_connection_from(struct Room* x) {
    if (x->numConnections < 6) {
        return true;
    } else {
        return false;
    }
}

/*******************************************************************************
 * Description: Returns true if the connection between the rooms already exists
 * and valse otherwise
 *
 * @param   struct Room*    x   room to check for connection
 * @param   struct Room*    y   room to check for connection 
 *
 * @return  enum bool           true if the rooms are connected and false 
 *                              otherwise
 *
*******************************************************************************/
enum bool connection_already_exists(struct Room* x, struct Room* y) {
    int i;
    // Check each room in x's outbound connections to see if it is room y.
    for (i = 0; i < x->numConnections; i++) {
        if (x->outboundConnections[i] == y) {
            return true;
        }
    }
    return false;
}

/*******************************************************************************
 * Description: Connect the two rooms by adding the addres of each to the
 * others outbound connections.
 *
 * @param   struct Room*    x   room connect
 * @param   struct Room*    y   room connect
 *
*******************************************************************************/
void connect_room(struct Room* x, struct Room* y) {
    // Add room y to room x's outbound connections and increment room x's 
    // connections.
    x->outboundConnections[x->numConnections] = y;
    x->numConnections++;
    return;
}

/*******************************************************************************
 * Description: Checks if the two passed rooms are the same room. 
 *
 * @param   struct Room*    x   room to check 
 * @param   struct Room*    y   room to check 
 *
 * @return  enum bool           true if the rooms are the same and false 
 *                              otherwise
 *
*******************************************************************************/
enum bool is_same_room(struct Room* x, struct Room* y) {
    if (x == y) {
        return true;
    } else {
        return false;
    }
}


/*******************************************************************************
 * Description: Makes a connection between two randomly chosen, connectable
 * rooms from roomList. 
 *
 * @param   struct Room*    roomList    the list of rooms. 
 *
*******************************************************************************/
void add_random_connection(struct Room* roomList) {
    struct Room* A;
    struct Room* B;

    // Choose random rooms for room A until one is found that has room for
    // a connection.
    while (true) {
        A = get_random_room(roomList);

        if(can_add_connection_from(A) == true) {
            break;
        }

    }

    // Choose random rooms for room B until one is found that has room for a
    // connection, isn't a duplicate of room A, and doesn't already have
    // a connection to room A.
    do {
        B = get_random_room(roomList);
    } while (can_add_connection_from(B) == false 
            || is_same_room(A,B) == true 
            || connection_already_exists(A, B) == true);

    connect_room(A, B);
    connect_room(B, A);

    return;
}

/*******************************************************************************
 * Description: Choose a random name for each room in the list from tne enum
 * RoomName.
 *
 * @param   struct Room*    roomList    the list of rooms. 
 *
*******************************************************************************/
void generate_random_names(struct Room* roomList) {
    enum bool usedNames[10];

    // Keep track of the list of used room names using each names corresponding
    // integer value.
    int i;
    for (i = 0; i < 10; i++) {
        usedNames[i] = false;
    }

    int  numNames = 0;

    // Keep choosing random names until a random name is chosen for each room
    // without collisions. Initialize other values for the room as well.
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

/*******************************************************************************
 * Description: Randomly choose a starting room and an ending room.
 *
 * @param   struct Room*    roomList    the list of rooms. 
 *
*******************************************************************************/
void choose_start_and_end(struct Room* roomList) {
    // Choose a random room to be the start room.
    int randomRoom = (rand() % (6 - 0 + 1));
    roomList[randomRoom].roomType = START_ROOM;

    // Keep choosing a random end room until a room is found that isn't the
    // start room.
    do {
        randomRoom = (rand() % (6 - 0 + 1));
    } while (roomList[randomRoom].roomType == START_ROOM);
    roomList[randomRoom].roomType = END_ROOM;
}


/*******************************************************************************
 * Description: Populate each room with a name, type, and list of connections. 
 *
 * @param   struct Room*    roomList    the list of rooms. 
 *
*******************************************************************************/
void initialize_room_list(struct Room* roomList) {
    // Generate room names, then choose start and end rooms.
    generate_random_names(roomList);
    // Keep adding connections until all rooms have at least three connections.
    choose_start_and_end(roomList);
    while (!is_graph_full(roomList)) {
        add_random_connection(roomList);
    }
    return;
}

/*******************************************************************************
 * Description: Write a room to a file. 
 *
 * @param   FILE*           ifp         filestream to write room to. 
 * @param   struct Room*    roomList    room to be written to file. 
 *
*******************************************************************************/
void write_room_to_file(FILE* ifp, struct Room* room) {
    // Write the room name to the filestream.
    fprintf(ifp, "ROOM NAME: %s\n", room_name_to_string(room->roomName));
    int i;
    // Write each room connection to the stream.
    for (i = 0; i < room->numConnections; i++) {
        fprintf(ifp, "CONNECTION %d: %s\n", 
                i + 1, 
                room_name_to_string(room->outboundConnections[i]->roomName));
    }
    // Write the type to the stream.
    fprintf(ifp, "ROOM TYPE: %s\n", room_type_to_string(room->roomType));
}

