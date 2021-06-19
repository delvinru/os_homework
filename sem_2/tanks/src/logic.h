#ifndef __LOGIC_H__
#define __LOGIC_H__

#include <pthread.h>
#include <stdint.h>
#include <ncurses.h>
#include <signal.h>
#include "game.h"

// Setup stuff
extern pthread_mutex_t mutex;
extern WINDOW* win;
extern WINDOW* mainWin;
extern window_size WindowSize;
extern volatile sig_atomic_t exitGame;
extern pthread_t tid2;
extern int PORT;
extern struct sockaddr_in IPs[2];

// Extern function
void* game_loop();
void move_player(int key, int PlayerID);
int update_status(int PlayerID);

// Game logic
#define TANK_TRACKS '#'
#define TANK_CENTER '*'
// #define TANK_BULLET ACS_BULLET
#define TANK_BULLET 'o'
#define MAX_BULLETS 5
#define MAX_PLAYERS 2
#define MAX_SPECTATORS 50
#define BULLET_TIME 50000

// Struct for describe position on screen
typedef struct Position {
    int32_t x;
    int32_t y;
} Position;

// Enum for guns postition
typedef enum GunPosition { 
    UP_LEFT,       
    UP_UP,             
    UP_RIGHT,           
    MIDDLE_LEFT,        
    MIDDLE_RIGHT,        
    DOWN_LEFT,          
    DOWN_DOWN,          
    DOWN_RIGHT    
} GunPosition;        

typedef enum MoveDirection{
    UP,
    DOWN,
    LEFT,
    RIGHT
} MoveDirection;

typedef struct Bullet{
    Position pos;
    Position direction;
    bool alive;
} Bullet;

// Struct for describe player object
typedef struct Player {
    // Define center position of tank
    Position pos;
    // Define move direction
    MoveDirection direction;
    // Define Gun Position
    GunPosition gun;
    // Define bullet
    Bullet bullet[MAX_BULLETS];
    // Count of bullets
    uint8_t amount_bullets;
    // Player Id
    uint8_t PlayerID;
} Player;

typedef struct Game{
    Player tanks[2];
} Game;

typedef struct Arguments{
    Player *tank;
    uint8_t playerId;
} Arguments;

// Function definitions
void* spawn_bullet(void* data);
void print_bullet(Player tank, int j);
void print_tank_borders(int PlayerID, bool blink);

// Current GameState
typedef struct GameState{
    char packetName[4];
    Game game;
} GameState;

#endif
