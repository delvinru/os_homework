#include <ncurses.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <time.h>
#include "snake.h"

BodyPoint* create_body(int x, int y)
{
    BodyPoint* point = calloc(1, sizeof(BodyPoint*));
    point->x = x;
    point->y = y;
    point->next = NULL;
    return point;
}

BodyPoint* create_snake()
{
    int init_x = COLS / 2;
    int init_y = LINES / 2;
    BodyPoint* head = create_body(init_x, init_y);
    BodyPoint* a = create_body(init_x+1, init_y);
    BodyPoint* b = create_body(init_x+1, init_y+1);
    BodyPoint* c = create_body(init_x+2, init_y+1);
    b->next = c;
    a->next = b;
    head->next = a;
    return head;
}

void add_new_food(Game* game)
{
    BodyPoint* list = game->fruits;
    while(list->next)
        list = list->next;
    list->next = create_body(rand() % COLS, rand() % LINES);
}

BodyPoint* create_fruits()
{
    BodyPoint* fruits = create_body(rand() % COLS, rand() % LINES);
    BodyPoint* a = create_body(rand() % COLS, rand() % LINES);
    fruits->next = a;
    for(int i = 0; i < 2; i++)
    {
        BodyPoint* b = create_body(rand() % COLS, rand() % LINES);
        a->next = b;
        a = b;
    }
    return fruits;
}

enum Direction get_move(enum Direction dir)
{
    int ch = getch();
    switch (ch)
    {
        case KEY_UP:
            if(dir != DOWN) return UP;
        case KEY_DOWN:
            if(dir != UP) return DOWN;
        case KEY_LEFT:
            if(dir != RIGHT) return LEFT;
        case KEY_RIGHT:
            if(dir != LEFT) return RIGHT;
        default:
            return dir;
    }
}

void display_snake(BodyPoint* snake)
{
    while(snake != NULL){
        mvaddch(snake->y, snake->x, POINT);
        snake = snake->next;
    }
}

void display_fruits(BodyPoint* fruits)
{
    while(fruits != NULL){
        mvaddch(fruits->y, fruits->x, ACS_DIAMOND);
        fruits = fruits->next;
    }
}

void display_score(unsigned int score)
{
    mvprintw(LINES-1, 1, "Score: %u\n", score);
}

void display_final_rating(unsigned int score)
{
    char str[128] = {0};
    sprintf(str, "FINAL RATING: %d\n", score);
    mvprintw(LINES/2, COLS/2 - strlen(str)/2, str, score);
}

BodyPoint* get_next_move(BodyPoint* snake, enum Direction dir)
{
    int x = snake->x;
    int y = snake->y;
    switch (dir)
    {
        case UP:
            y = snake->y - 1;
            break;
        case DOWN:
            y = snake->y + 1;
            break;
        case LEFT:
            x = snake->x - 1;
            break;
        case RIGHT:
            x = snake->x + 1;
            break;
    }
    if(x < 0)
        x = COLS;
    if(x > COLS)
        x = 0;
    if(y < 0)
        y = LINES;
    if(y > LINES)
        y = 0;
    return create_body(x, y);
}

int check_food_colision(BodyPoint* snake, BodyPoint* fruits)
{
    BodyPoint* list = fruits;
    while(list)
    {
        if(snake->x == list->x && snake->y == list->y)
            return true;
        list = list->next;
    }
    return false;
}

int check_snake_colision(BodyPoint* snake, BodyPoint* head)
{
    BodyPoint* list = snake;
    while(list)
    {
        if(list->x == head->x && list->y == head->y)
            return true;
        list = list->next;
    }
    return false;
}

void remove_food_from_list(BodyPoint* head, BodyPoint** fruits)
{
    BodyPoint* list = *fruits;
    BodyPoint* prev = NULL;
    while(list)
    {
        if(head->x == list->x && head->y == list->y)
        {
            if(prev == NULL)
                *fruits = list->next;
            else
                prev->next = list->next;
            free(list);
            break;
        }

        prev = list;
        list = list->next;
    }
}

BodyPoint* move_snake(Game* game, enum Direction dir)
{
    BodyPoint* snake = game->snake;
    BodyPoint* head = get_next_move(snake, dir);

    if(check_snake_colision(snake, head))
    {
        display_final_rating(game->score);
        return NULL;
    }

    head->next = snake;
    snake = head;

    // Check if head contain fruits position
    if(check_food_colision(snake, game->fruits))
    {
        remove_food_from_list(snake, &game->fruits);
        add_new_food(game);
        game->score += 100;
        return snake;
    }

    // Remove end
    BodyPoint* tail = snake;

    while(tail->next->next != NULL)
        tail = tail->next;

    free(tail->next);
    tail->next = NULL;

    return snake;
}

Game* init_game()
{
    Game* game = calloc(1, sizeof(*game));
    BodyPoint *snake = create_snake();
    BodyPoint *fruits = create_fruits();
    game->snake = snake;
    game->fruits = fruits;
    game->score = 0;
    return game;
}

int main(void)
{
    // Init screen and other stuff
    initscr();
    keypad(stdscr, TRUE);           // correct keys
    noecho();
    curs_set(0);                    // hide cursor
    timeout(100);
    srand(time(NULL));

    start_color();
    init_pair(1, COLOR_GREEN, COLOR_BLACK);
    init_pair(2, COLOR_RED, COLOR_BLACK);
    init_pair(3, COLOR_YELLOW, COLOR_BLACK);

    Game* game = init_game();
    enum Direction dir = UP;

    while(true)
    {
        clear();
        attron(COLOR_PAIR(1));
        display_snake(game->snake);
        attroff(COLOR_PAIR(1));

        attron(COLOR_PAIR(2));
        display_fruits(game->fruits);
        attroff(COLOR_PAIR(2));

        attron(COLOR_PAIR(3));
        display_score(game->score);

        refresh();
        dir = get_move(dir);
        game->snake = move_snake(game, dir);
        attroff(COLOR_PAIR(3));
        if(game->snake == NULL)
            break;
        game->score += 1;
    }
    // Close ncurses window
    timeout(-1);
    getch();
    endwin();

    return 0;
}