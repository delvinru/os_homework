#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <ncurses.h>

#include "banker.h"

WINDOW *win = NULL;
WindowSize MainWindow;
Data data;
Point T_position[4];
Point R_position[4][4];
int counter = 0;

int main(void)
{
    // Initialize ncurses window
    init_window();
    // Initialize data
    init_data_structure();
    int result = true;
    while (result == true)
    {
        if (counter != 0)
            clear_window();
        // Draw begin state
        draw_init_state();
        // Get input from user
        input_data();
        // Process algorithm
        result = bankir_algorithm();
        counter++;
    }

    // Restore terminal
    endwin();
    return 0;
}

void clear_window(void)
{
    wattr_on(win, COLOR_PAIR(BGC_COLOR), NULL);
    mvwprintw(win, MainWindow.height / 2, (MainWindow.width - 10) / 2, "          ");
    mvwprintw(win, MainWindow.height / 2 + 1, (MainWindow.width - 36) / 2, "                                   ");
    wattr_off(win, COLOR_PAIR(BGC_COLOR), NULL);
}

int matrix_R_is_zero(void)
{
    int flag = true;
    for (int i = 0; i < 4; i++)
        for (int j = 0; j < 4; j++)
            if (data.R[i][j] != 0)
                flag = false;
    return flag;
}

int has_empty_row_R(void)
{
    int flag = true;
    for (int i = 0; i < 4; i++)
    {
        for (int j = 0; j < 4; j++)
        {
            if (data.R[i][j] != 0)
            {
                flag = false;
                break;
            }
            else
                flag = true;
        }
        if (flag)
            return flag;
    }
    return flag;
}

int bankir_algorithm(void)
{
    curs_set(0);
    int quarterX = MainWindow.width * 0.25;
    int quarterY = MainWindow.height * 0.25;
    int offset = (4 + 12) / 2;

    if (counter == 0)
    {
        for (int i = 0; i < 4; i++)
            data.A[i] = data.T[i];
    }

    int flag = true;
    // Repeat 16 times for every possible line
    for (int i = 0; i < 4 * 4; i++)
    {
        // Check if we have empty row or not
        if (i != 0 && i % 4 == 0)
        {
            if (has_empty_row_R() == false)
            {
                flag = false;
                break;
            }
        }

        // If matrix is empty
        if (i != 0 && (i % 4) == 0 && matrix_R_is_zero() == true)
        {
            wattr_on(win, COLOR_PAIR(STATE_COLOR), NULL);
            mvwprintw(win, quarterY * 3 + 0 - 2, quarterX * 3 + 4 - offset, "( %d %d %d %d )", data.R[i % 4][0], data.R[i % 4][1], data.R[i % 4][2], data.R[i % 4][3]);
            mvwprintw(win, quarterY * 3 + 1 - 2, quarterX * 3 + 4 - offset, "( %d %d %d %d )", data.R[i % 4][0], data.R[i % 4][1], data.R[i % 4][2], data.R[i % 4][3]);
            mvwprintw(win, quarterY * 3 + 2 - 2, quarterX * 3 + 4 - offset, "( %d %d %d %d )", data.R[i % 4][0], data.R[i % 4][1], data.R[i % 4][2], data.R[i % 4][3]);
            mvwprintw(win, quarterY * 3 + 3 - 2, quarterX * 3 + 4 - offset, "( %d %d %d %d )", data.R[i % 4][0], data.R[i % 4][1], data.R[i % 4][2], data.R[i % 4][3]);
            wattr_off(win, COLOR_PAIR(STATE_COLOR), NULL);
            wrefresh(win);
            sleep(1);
            wattr_on(win, COLOR_PAIR(BGC_COLOR), NULL);
            mvwprintw(win, quarterY * 3 + 0 - 2, quarterX * 3 + 4 - offset, "( %d %d %d %d )", data.R[i % 4][0], data.R[i % 4][1], data.R[i % 4][2], data.R[i % 4][3]);
            mvwprintw(win, quarterY * 3 + 1 - 2, quarterX * 3 + 4 - offset, "( %d %d %d %d )", data.R[i % 4][0], data.R[i % 4][1], data.R[i % 4][2], data.R[i % 4][3]);
            mvwprintw(win, quarterY * 3 + 2 - 2, quarterX * 3 + 4 - offset, "( %d %d %d %d )", data.R[i % 4][0], data.R[i % 4][1], data.R[i % 4][2], data.R[i % 4][3]);
            mvwprintw(win, quarterY * 3 + 3 - 2, quarterX * 3 + 4 - offset, "( %d %d %d %d )", data.R[i % 4][0], data.R[i % 4][1], data.R[i % 4][2], data.R[i % 4][3]);
            wattr_off(win, COLOR_PAIR(BGC_COLOR), NULL);
            wrefresh(win);
            sleep(1);
            flag = true;
            break;
        }

        wattr_on(win, COLOR_PAIR(STATE_COLOR), NULL);
        mvwprintw(win, quarterY, quarterX * 3 + 4 - offset, "( %d %d %d %d )", data.A[0], data.A[1], data.A[2], data.A[3]);
        mvwprintw(win, quarterY * 3 + (i % 4) - 2, quarterX * 3 + 4 - offset, "( %d %d %d %d )", data.R[i % 4][0], data.R[i % 4][1], data.R[i % 4][2], data.R[i % 4][3]);
        wattr_off(win, COLOR_PAIR(STATE_COLOR), NULL);
        wrefresh(win);
        sleep(1);

        // Process have nessecary resourses
        if (
            data.R[i % 4][0] <= data.A[0] &&
            data.R[i % 4][1] <= data.A[1] &&
            data.R[i % 4][2] <= data.A[2] &&
            data.R[i % 4][3] <= data.A[3])
        {
            if (counter % 2 == 0)
            {
                // Process have enough memory for init
                for (int j = 0; j < 4; j++)
                {
                    data.C[i % 4][j] = data.R[i % 4][j];
                    data.A[j] -= data.R[i % 4][j];
                    data.R[i % 4][j] = 0;
                }
            }
            else
            {
                for (int j = 0; j < 4; j++)
                {
                    data.A[j] += data.C[i % 4][j];
                    data.C[i % 4][j] = 0;
                    data.R[i % 4][j] = 0;
                }
            }

            wattr_on(win, COLOR_PAIR(STATE_COLOR), NULL);
            mvwprintw(win, quarterY, quarterX * 3 + 4 - offset, "( %d %d %d %d )", data.A[0], data.A[1], data.A[2], data.A[3]);
            wattr_off(win, COLOR_PAIR(STATE_COLOR), NULL);

            wattr_on(win, COLOR_PAIR(SUCCESS_COLOR), NULL);
            mvwprintw(win, quarterY * 3 + i % 4 - 2, quarterX + 4 - offset, "( %d %d %d %d )", data.C[i % 4][0], data.C[i % 4][1], data.C[i % 4][2], data.C[i % 4][3]);
            mvwprintw(win, quarterY * 3 + i % 4 - 2, quarterX * 3 + 4 - offset, "( %d %d %d %d )", data.R[i % 4][0], data.R[i % 4][1], data.R[i % 4][2], data.R[i % 4][3]);
            wattr_off(win, COLOR_PAIR(SUCCESS_COLOR), NULL);
        }
        else
        {
            // Process don't have enough memory
            flag = false;
            wattr_on(win, COLOR_PAIR(ERROR_COLOR), NULL);
            mvwprintw(win, quarterY * 3 + i % 4 - 2, quarterX + 4 - offset, "( %d %d %d %d )", data.C[i % 4][0], data.C[i % 4][1], data.C[i % 4][2], data.C[i % 4][3]);
            mvwprintw(win, quarterY * 3 + i % 4 - 2, quarterX * 3 + 4 - offset, "( %d %d %d %d )", data.R[i % 4][0], data.R[i % 4][1], data.R[i % 4][2], data.R[i % 4][3]);
            wattr_off(win, COLOR_PAIR(ERROR_COLOR), NULL);
        }

        wrefresh(win);
        sleep(1);

        wattr_on(win, COLOR_PAIR(BGC_COLOR), NULL);
        mvwprintw(win, quarterY * 3 + i % 4 - 2, quarterX + 4 - offset, "( %d %d %d %d )", data.C[i % 4][0], data.C[i % 4][1], data.C[i % 4][2], data.C[i % 4][3]);
        mvwprintw(win, quarterY * 3 + i % 4 - 2, quarterX * 3 + 4 - offset, "( %d %d %d %d )", data.R[i % 4][0], data.R[i % 4][1], data.R[i % 4][2], data.R[i % 4][3]);
        wattr_off(win, COLOR_PAIR(BGC_COLOR), NULL);
        wrefresh(win);
        sleep(1);
    }

    mvwprintw(win, quarterY, quarterX * 3 + 4 - offset, "( %d %d %d %d )", data.A[0], data.A[1], data.A[2], data.A[3]);
    wrefresh(win);

    if (flag)
    {
        wattr_on(win, COLOR_PAIR(SUCCESS_COLOR), NULL);
        mvwprintw(win, MainWindow.height / 2, (MainWindow.width - 10) / 2, "All fine!");
        mvwprintw(win, MainWindow.height / 2 + 1, (MainWindow.width - 36) / 2, "Press any key to input new data ...");
        wattr_off(win, COLOR_PAIR(SUCCESS_COLOR), NULL);
    }
    else
    {
        wattr_on(win, COLOR_PAIR(ERROR_COLOR), NULL);
        mvwprintw(win, MainWindow.height / 2, (MainWindow.width - 10) / 2, "Blocking!");
        mvwprintw(win, MainWindow.height / 2 + 1, (MainWindow.width - 26) / 2, "Press any key to exit ...");
        wattr_off(win, COLOR_PAIR(ERROR_COLOR), NULL);
    }
    wrefresh(win);
    // Wait until any button pressed
    wgetch(win);
    return flag;
}

int check_input_data(void)
{
    int flag = true;
    for (int i = 0; i < 4; i++)
    {
        if (data.T[i] == -1)
        {
            flag = false;
            break;
        }
        for (int j = 0; j < 4; j++)
        {
            if (data.R[i][j] == -1)
            {
                flag = false;
                break;
            }
        }
    }
    return flag;
}

void set_value(Point cur_pos, int value)
{
    for (int i = 0; i < 4; i++)
    {
        if (cur_pos.x == T_position[i].x && cur_pos.y == T_position[i].y)
        {
            data.T[i] = value;
            break;
        }
        for (int j = 0; j < 4; j++)
        {
            if (cur_pos.x == R_position[i][j].x && cur_pos.y == R_position[i][j].y)
            {
                data.R[i][j] = value;
                break;
            }
        }
    }
}

Point get_new_position(Point cur_pos, int dir)
{
    if (counter == 0)
    {
        // Check if we in T_position
        for (int i = 0; i < 4; i++)
        {
            if (cur_pos.x == T_position[i].x && cur_pos.y == T_position[i].y)
            {
                switch (dir)
                {
                case KEY_UP:
                    break;

                case KEY_RIGHT:
                    if (i == 3)
                        return R_position[0][0];
                    return T_position[i + 1];
                    break;

                case KEY_LEFT:
                    if (i == 0)
                        return cur_pos;
                    return T_position[i - 1];
                    break;

                case KEY_DOWN:
                    return R_position[0][i];
                    break;

                default:
                    break;
                }
            }
        }
    }
    // Check if we in R_position
    for (int i = 0; i < 4; i++)
    {
        for (int j = 0; j < 4; j++)
        {
            if (cur_pos.x == R_position[i][j].x && cur_pos.y == R_position[i][j].y)
            {
                switch (dir)
                {
                case KEY_UP:
                    if (i == 0 && counter == 0)
                        return T_position[j];
                    if (i == 0)
                        return cur_pos;
                    return R_position[i - 1][j];
                    break;

                case KEY_DOWN:
                    if (i == 3)
                        return cur_pos;
                    return R_position[i + 1][j];
                    break;

                case KEY_LEFT:
                    if (i == 0 && j == 0 && counter == 0)
                        return T_position[3];
                    if (i == 0 && j == 0 && counter != 0)
                        return R_position[0][0];
                    if (j == 0)
                        return R_position[i - 1][3];
                    return R_position[i][j - 1];
                    break;

                case KEY_RIGHT:
                    if (i == 3 && j == 3)
                        return cur_pos;
                    if (j == 3)
                        return R_position[i + 1][0];
                    return R_position[i][j + 1];
                    break;

                default:
                    break;
                }
            }
        }
    }
}

void input_data()
{
    curs_set(1);

    int ch, check = true;
    int value = 0;

    Point cur_pos;

    if (counter == 0)
        cur_pos = T_position[0];
    else
        cur_pos = R_position[0][0];

    wmove(win, cur_pos.y, cur_pos.x);

    while (check)
    {
        ch = wgetch(win);
        switch (ch)
        {
        case 'q':
            delwin(win);
            endwin();
            exit(0);
            break;
        case KEY_RIGHT:
            cur_pos = get_new_position(cur_pos, KEY_RIGHT);
            break;
        case KEY_LEFT:
            cur_pos = get_new_position(cur_pos, KEY_LEFT);
            break;
        case KEY_UP:
            cur_pos = get_new_position(cur_pos, KEY_UP);
            break;
        case KEY_DOWN:
            cur_pos = get_new_position(cur_pos, KEY_DOWN);
            break;

        case '0':
        case '1':
        case '2':
        case '3':
        case '4':
        case '5':
        case '6':
        case '7':
        case '8':
        case '9':
            mvwprintw(win, cur_pos.y, cur_pos.x, "%c", ch);
            set_value(cur_pos, ch - 0x30);
            cur_pos = get_new_position(cur_pos, KEY_RIGHT);
            break;

        case ' ':
            check = !check_input_data();
            break;

        default:
            break;
        }
        wmove(win, cur_pos.y, cur_pos.x);
    }
    curs_set(0);
}

void init_data_structure(void)
{
    for (int i = 0; i < 4; i++)
    {
        for (int j = 0; j < 4; j++)
        {
            data.C[i][j] = 0;
            data.R[i][j] = -1;
        }
        data.A[i] = 0;
        data.T[i] = -1;
    }
}

void draw_init_state()
{
    int quarterX = MainWindow.width * 0.25;
    int quarterY = MainWindow.height * 0.25;
    int offset = (4 + 12) / 2;

    if (counter == 0)
    {
        // Draw vector T
        mvwprintw(win, quarterY, quarterX - offset, "T = ");
        mvwprintw(win, quarterY, quarterX + 4 - offset, "( _ _ _ _ )");
    }

    // Init cursor position for vector T
    for (int i = 0; i < 4; i++)
    {
        T_position[i].x = quarterX - offset + 4 + 2 + i * 2;
        T_position[i].y = quarterY;
    }

    // Draw vector A
    mvwprintw(win, quarterY, quarterX * 3 - offset, "A = ");
    mvwprintw(win, quarterY, quarterX * 3 + 4 - offset, "( %d %d %d %d )", data.A[0], data.A[1], data.A[2], data.A[3]);

    // Draw matrix C
    mvwprintw(win, quarterY * 3, quarterX - offset, "C = ");
    for (int i = 0; i < 4; i++)
        mvwprintw(win, quarterY * 3 + i - 2, quarterX + 4 - offset, "( %d %d %d %d )", data.C[i][0], data.C[i][1], data.C[i][2], data.C[i][3]);

    // Draw matrix R
    mvwprintw(win, quarterY * 3, quarterX * 3 - offset, "R = ");
    for (int i = -2; i < 2; i++)
        mvwprintw(win, quarterY * 3 + i, quarterX * 3 + 4 - offset, "( _ _ _ _ )");

    // Init cursor position for matrix R
    for (int i = 0; i < 4; i++)
    {
        for (int j = 0; j < 4; j++)
        {
            R_position[i][j].x = quarterX * 3 + 4 - offset + 2 + j * 2;
            R_position[i][j].y = quarterY * 3 + i - 2;
        }
    }
}

void init_window(void)
{
    initscr();
    noecho();
    curs_set(0);
    keypad(stdscr, true);

    // Check if terminal support colors
    if (!has_colors())
    {
        endwin();
        perror("Terminal doesn't support colors");
        exit(0);
    }

    // Init color pairs
    start_color();
    init_pair(BGC_COLOR, COLOR_BLACK, COLOR_CYAN);
    init_pair(ERROR_COLOR, COLOR_WHITE, COLOR_RED);
    init_pair(SUCCESS_COLOR, COLOR_BLACK, COLOR_GREEN);
    init_pair(STATE_COLOR, COLOR_BLACK, COLOR_YELLOW);

    int height = LINES * 0.5;
    int width = COLS * 0.5;

    MainWindow.width = (int)width;
    MainWindow.height = (int)height;

    // Init window
    win = newwin(height, width, LINES * 0.25, COLS * 0.25);

    // Correct keyboard inputs
    keypad(win, true);

    // Set borders for window
    box(win, 0, 0);

    // Set title for box
    mvwprintw(win, 0, (width - 21) / 2, "%s", " Banker's algorithm ");

    // Fill background with BGC_COLOR
    wbkgd(win, COLOR_PAIR(BGC_COLOR));
}