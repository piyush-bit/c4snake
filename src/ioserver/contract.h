#ifndef CONTRACT_H
#define CONTRACT_H

// use of the contract.h
// take in input from multiple source
// when the tick happens take the snapshot and relay every other source
// relay engine generated states i.e food and wall

#include <string.h>

enum Commands {
    UP='w',
    DOWN='s',
    LEFT='a',
    RIGHT='d'
};

// TODO: define the type of message and it's contract
// for now it is of  types player_dir(playerid,direction) , foodspwan(x,y) , playerspwan(x,y,playerid)

enum MessageType {
    PLAYERDIR,
    FOODSPWAN,
    PLYERSPWAN
};

struct Message{
    enum MessageType messageType;
    char* data;
};

// generalized interface to contact the player
struct PlayerConnection{
    int connectionType;
    void* connectionPointer;
};

struct Playerinfo{
    char name[50];
    int playerType;
    char command;
    struct PlayerConnection connection;
    void (*readAndWriteCommand)(struct PlayerConnection conn , char* command); // needs to be a blocking function otherwise the cpu usage goes 100%
    int (*FrameAndSendMessage)(struct PlayerConnection conn ,struct Message message);
};

struct ContractInfo {
    size_t capacity ;
    struct Playerinfo **playerinfo;
    char* filled; // represent filled status , across spwan and death
    char *snapshots; // saves the all the players command for transmission per tick
};

// for read , we do for each
// while(1){pli->readAndWriteCommand(pli->conn,&pli->comd)}
// for writing command we just push in the FrameAndSend , handling of buffer and transport retry logic by the handler however they like , they can stuff any number of thing connectinPointer and use that

struct ContractInfo* initIoServerContract(size_t capacity);


#endif
