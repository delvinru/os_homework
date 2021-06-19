#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <ncurses.h>

WINDOW *win = NULL;

void sum_cols(const int C[4][4], int* A)
{
    for(int i = 0; i < 4; i++)
    {
        int tmp = 0;
        for(int j = 0; j < 4; j++)
            tmp += C[j][i];
        A[i] = 9 - tmp;
    }
}

void print_matrix(const int C[4][4], int y, int x, const char* name)
{
    int tmp = x;
    mvwprintw(win, y - 1, x+3, "%s", name);
    for(int i = 0; i < 4; i++)
    {
        x = tmp;
        for(int j = 0; j < 4; j++)
        {
            mvwprintw(win, y, x, "%d ", C[i][j]);
            x += 2;
        }
        y++;
    }
}

void print_row(const int A[4], int y, int x, const char* name)
{
    mvwprintw(win, y-1, x + 3, name);
    for(int i = 0; i < 4; i++)
    {
        mvwprintw(win, y, x, "%d ", A[i]);
        x += 2;
    }
}

int check[4] = {1, 1, 1, 1};

int do_magic(int C[4][4], int R[4][4], int A[4])
{
    int zero = 0;

    for(int i = 0; i < 4; i++)
    {
        zero = 0;
        
        for(int j = 0; j < 4 && check[i]; j++)
        {
            if(R[i][j] > A[j])
                zero = 1;
        }
        
        if(zero == 0 && check[i])
        {
            for(int j = 0; j < 4; j++)
            {
                R[i][j] = 0;
                A[j] += C[i][j];
            }
            check[i] = 0;
        }
        
        print_matrix(R, 10, 50, "R");
        print_row(A, 5, 50, "A");
        wrefresh(win);
    }

    return 1;
}

int main(int argc, char**argv)
{
    // Init T
    int T[4] = {9, 9, 9, 9};
    int C[4][4] = {{2, 3, 1, 0}, {2, 5, 1, 1}, {1, 0, 2, 1}, {0, 0, 0, 1}};

    int R[4][4] = {{3, 2, 1, 2}, {2, 0, 5, 6}, {2, 1, 7, 2}, {1, 0, 3, 2}};

    int A[4] = {0, 0, 0, 0};    

    if(argc > 1)
        R[2][2] = 8;

    sum_cols(C, A); 
    
    // Run ncurses init stuff
    initscr();
    noecho();
    keypad(stdscr, TRUE);
    timeout(100);
    curs_set(0);

    int height = LINES*0.8;
    int width = COLS*0.8;

    win = newwin(height, width, LINES*0.1, COLS*0.1);
    refresh();

    print_matrix(C, 10, 10, "C");
    print_matrix(R, 10, 50, "R");
    print_row(A, 5, 50, "A");
    wrefresh(win);

    for(int i = 0; i < 4; i++)
    {
        wclear(win);
        box(win, 0, 0); 
        print_row(T, 5, 10, "T");
        print_matrix(C, 10, 10, "C");

        do_magic(C, R, A);

        sleep(1);
        wrefresh(win);
    }

    int tmp = 0;
    for(int i = 0; i < 4; i++)
        tmp += check[i];

    if(tmp == 0)
    {
        char* str = "No blokirovka"; 
        mvwprintw(win, height/2, (width - strlen(str))/2, "%s", str);
    } else
    {
        char* str = "Blokirovka"; 
        mvwprintw(win, height/2, (width - strlen(str))/2, "%s", str);
    }
    
    wrefresh(win);
    
    wgetch(win);
    // Endwin
    endwin();
    return 0;
}
