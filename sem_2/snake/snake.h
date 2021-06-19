#ifndef SNAKE_H
#define SNAKE_H

#include <ncurses.h>

typedef struct BodyPoint{
    int32_t x;
    int32_t y;
    struct BodyPoint* next;
} BodyPoint;


typedef struct Game{
    BodyPoint* snake;
    BodyPoint* fruits;
    unsigned int score;
} Game;

BodyPoint* create_body(int x, int y);
BodyPoint* create_snake();
enum Direction get_move(enum Direction dir);
void display_snake(BodyPoint* snake);
BodyPoint* get_next_move(BodyPoint* snake, enum Direction dir);
Game* init_game();


enum Direction {UP, DOWN, LEFT, RIGHT};

#define POINT '#'

#endif