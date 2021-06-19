#ifndef _MAIN_H_
#define _MAIN_H_

typedef struct WindowSize {
    int width;
    int height;
} WindowSize;

typedef struct Data {
    int C[4][4];
    int T[4];
    int A[4];
    int R[4][4];
} Data;

typedef struct Point {
    int y;
    int x;
} Point;


void init_window(void);
void draw_init_state(void);
void init_data_structure(void);
void clear_window(void);

void input_data(void);
void set_value(Point cur_pos, int value);
Point get_new_position(Point cur_pos, int dir);
int check_input_data(void);

int bankir_algorithm(void);

#define BGC_COLOR 1
#define ERROR_COLOR 2
#define SUCCESS_COLOR 3
#define STATE_COLOR 4

#endif
