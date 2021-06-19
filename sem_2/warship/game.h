#ifndef GAME
#define GAME

// Game object
#define SHIP_LENGTH 5
#define SHIP_BODY '#'
#define MAX_BULLETS 3
#define BULLET_BODY '*'

typedef struct Window_stat {
    int width;
    int height;
    int top;
} Window_stat;

typedef struct Cannon {
    int y;
    int x;
    int bullets;
} Cannon;

typedef struct Ship {
    int y[SHIP_LENGTH];
    int x;
    int len;
} Ship;

typedef struct Bullet{
    int y;
    int x;
} Bullet;

// Functions
void init_window();
void win_function();

// Main threads
static void* game_loop();
static void* ship_loop();
static void* spawn_bullet();

// Helpful functions
void draw_cannon();
int catch_input();
void draw_ship();
void move_ship();

#endif