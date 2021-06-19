#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <ncurses.h>

#include "game.h"

/*
GAME PLAN:

1. Create 3 threads
    * 1 thread - draw game
    * 2 thread - draw bullets
    * 3 thread - draw ship
2. Game rule
    * Ship move from bottom to up in right side of window
    * No more than 3 bullets on screen
    * If the bullet hit the ship, then the end of the game
3. Possible diffuculties
    * Check the `hit` moment
*/

WINDOW *win;
Window_stat game_window;
Cannon cannon;
Ship ship;

pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

int main(void)
{
    // Just setup window
    init_window();

    // Start threads
    pthread_t pid[2];
    pthread_create(&pid[0], NULL, game_loop, NULL);
    pthread_create(&pid[1], NULL, ship_loop, NULL);

    pthread_join(pid[0], NULL);
    pthread_join(pid[1], NULL);

    endwin();
    return 0;
}

// Draws borders and cannon
static void* game_loop()
{
    // Init cannon
    cannon.bullets = 0;
    cannon.y = game_window.height / 2;
    cannon.x = 1;

    // Draw cannon - static object
    draw_cannon();
    refresh();
    box(win, 0, 0);

    // Init bullets threads
    pthread_t bullets[MAX_BULLETS];
    while(true)
    {
        pthread_mutex_lock(&mutex);
        wrefresh(win);
        pthread_mutex_unlock(&mutex);
        if(catch_input())
        {
            pthread_mutex_lock(&mutex);
            if(cannon.bullets < MAX_BULLETS)
            {
                pthread_create(&bullets[cannon.bullets], NULL, spawn_bullet, NULL);
                cannon.bullets += 1;
            }
            pthread_mutex_unlock(&mutex);
        }
        pthread_mutex_lock(&mutex);
        mvprintw(LINES-2, 3, "Current bullets: %d", cannon.bullets);
        pthread_mutex_unlock(&mutex);
    }
    // attroff(COLOR_PAIR(1));
    return NULL;
}

void draw_cannon()
{
    wattron(win, COLOR_PAIR(3));
    pthread_mutex_lock(&mutex);
    mvwaddch(win, cannon.y - 1, cannon.x, '=');
    for(int i = 0; i < 3; i++)
        mvwaddch(win, cannon.y, cannon.x + i, '=');
    mvwaddch(win, cannon.y + 1, cannon.x, '=');
    pthread_mutex_unlock(&mutex);
    wattroff(win, COLOR_PAIR(3));
}

int catch_input()
{
    int ch = getch();
    if(ch == ' ')
        return 1;
    return 0;
}

static void* ship_loop()
{
    ship.x = game_window.width - 3;
    ship.y[0] = (int)(game_window.height / 2);
    for(int i = 1; i < SHIP_LENGTH; i++)
        ship.y[i] = ship.y[0] + i;

    ship.len = SHIP_LENGTH;

    while(true)
    {
        move_ship();
        draw_ship();
        // usleep(500000);
        usleep(100000);
    }

    return NULL;
}

void move_ship()
{
    for(int i = 0; i < SHIP_LENGTH; i++)
    {
        ship.y[i] -= 1;
        // Check border escape
        if(ship.y[i] > game_window.height - 2)
            ship.y[i] = 1;
        else if(ship.y[i] < 1)
            ship.y[i] = game_window.height - 2;
    }
}

void draw_ship()
{
    wattron(win, COLOR_PAIR(4));
    pthread_mutex_lock(&mutex);
    for(int i = 0; i < SHIP_LENGTH; i++)
        mvwaddch(win, ship.y[i], ship.x, SHIP_BODY);
    mvwaddch(win, ship.y[SHIP_LENGTH-1], ship.x, ' ');
    pthread_mutex_unlock(&mutex);
    wattroff(win, COLOR_PAIR(4));
}

static void* spawn_bullet()
{
    Bullet bullet;
    bullet.y = game_window.height / 2;
    bullet.x = 5;
    while(true)
    {
        // Draw new bullet
        wattron(win, COLOR_PAIR(2));
        pthread_mutex_lock(&mutex);
        mvwaddch(win, bullet.y, bullet.x, BULLET_BODY);
        pthread_mutex_unlock(&mutex);

        pthread_mutex_lock(&mutex);
        for(int i = 0; i < SHIP_LENGTH; i++)
            if(bullet.y == ship.y[i] && bullet.x == ship.x)
                win_function();
        pthread_mutex_unlock(&mutex);

        if(bullet.x > game_window.width - 3)
        {
            pthread_mutex_lock(&mutex);
            mvwaddch(win, bullet.y, bullet.x - 1, ' ');
            mvwaddch(win, bullet.y, bullet.x, ' ');
            pthread_mutex_unlock(&mutex);
            break;
        }
        // Clear previous position
        pthread_mutex_lock(&mutex);
        bullet.x += 1;
        mvwaddch(win, bullet.y, bullet.x - 2, ' ');
        pthread_mutex_unlock(&mutex);
        usleep(50000);
        wattroff(win, COLOR_PAIR(2));
    }
    pthread_mutex_lock(&mutex);
    cannon.bullets -= 1;
    pthread_mutex_unlock(&mutex);
    return NULL;
}

void win_function()
{
    int y = (int)(LINES/2);
    char *str1 = "WINNER WINNER";
    char *str2 = "CHICKEN DINNER!";
    mvwprintw(stdscr, y, (COLS-14)/2, "%s", str1);
    mvwprintw(stdscr, y+1, (COLS-16)/2, "%s", str2);

    nodelay(win, false);
    nodelay(stdscr, false);

    getch();
    
    endwin();
    exit(0);
}

void init_window()
{
    // Init screen
    initscr();
    // Correct keypad
    keypad(stdscr, TRUE);
    // Delay for drawing
    timeout(100);
    // Don't print charachter
    noecho();
    // Hide cursor
    curs_set(0);
    // Init colors
    start_color();
    // For background
    init_pair(1, COLOR_WHITE, COLOR_BLUE);
    // For bullet
    init_pair(2, COLOR_RED, COLOR_BLUE);
    // For cannon
    init_pair(3, COLOR_BLACK, COLOR_BLUE);
    // For ship
    init_pair(4, COLOR_BLACK, COLOR_BLUE);

    int height = (int)((float)LINES * 0.8);
    int width = (int)((float)COLS * 0.5);

    game_window.height = height;
    game_window.width = width;
    game_window.top = (int)((float)LINES*0.1);

    win = newwin(height, width, (int)((float)LINES*0.1), (int)((float)COLS*0.25));
    wbkgd(win, COLOR_PAIR(1));
}
