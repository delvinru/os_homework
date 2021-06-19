#ifndef __GAME_H__
#define __GAME_H__

#include <pthread.h>

typedef struct window_size{
    int height;
    int width;
} window_size;

void init_window(void);
void close_game(int signum);

#endif