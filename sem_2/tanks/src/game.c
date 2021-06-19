#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>
#include <pthread.h>
#include <ncurses.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/select.h>
#include <string.h>
#include <signal.h>

#include "server.h"
#include "logic.h"
#include "game.h"

// Default port
int PORT = 8888;

// Define global variable
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
WINDOW *win = NULL;
WINDOW *mainWin = NULL;
window_size WindowSize;

// Threads
pthread_t tid1, tid2;

// Struct for players IP
struct sockaddr_in IPs[2];

// Game loop check
volatile sig_atomic_t exitGame = 0;

int main(int argc, char **argv)
{
    // Check arguments
    if (argc < 3)
    {
        printf(
            "Usage: %s <ip1> <ip2>\n"
            "\t<ip1> - \033[31mPlayer 1\033[0m\n"
            "\t<ip2> - \033[34mPlayer 2\033[0m\n"
            "A blinking block indicates that the player has not yet accepted the game\n"
            "Good luck!\n",
            argv[0]);
        return 0;
    }

    if (!strcmp(argv[1], argv[2]))
    {
        puts("You entered the same ip's! How did you plan to play?");
        exit(1);
    }

    // Convert ip to struct for next work
    inet_aton(argv[1], &IPs[0].sin_addr);
    inet_aton(argv[2], &IPs[1].sin_addr);

    // Setup ncurses window
    init_window();

    // Registrer handler for C-c and exit
    signal(SIGINT, close_game);

    // Run game loop
    pthread_create(&tid1, NULL, game_loop, NULL);
    // Run servers thread
    pthread_create(&tid2, NULL, udp_server, NULL);

    pthread_join(tid1, NULL);
    pthread_join(tid2, NULL);

    // Close ncurses
    delwin(mainWin);
    delwin(win);
    endwin();

    // Print final message and close game
    fprintf(stderr, "Good bye!\n");
    return 0;
}

void init_window(void)
{
    // Init screen
    initscr();
    // Correct keypad
    keypad(stdscr, TRUE);
    // Setup window size
    int row = 40;
    int col = 100;

    // Check screen size
    if (LINES < row && COLS < col)
    {
        endwin();
        fprintf(stderr, "Your size: LINES %d, COLS: %d\n", LINES, COLS);
        fprintf(stderr, "Sorry not enough terminal screen size\n");
        fprintf(stderr, "Need LINES >= %d and COLS >= %d\n", row, col);
        exit(1);
    }
    // Don't print charachter
    noecho();
    // Hide cursor
    curs_set(0);
    if (false == has_colors())
    {
        perror("Terminal don't support colors!");
        endwin();
        exit(1);
    }
    // Init colors
    start_color();
    // Title
    init_pair(1, COLOR_BLACK, COLOR_GREEN);
    // Player 1
    init_pair(2, COLOR_RED, COLOR_BLACK);
    // Player 2
    init_pair(3, COLOR_BLUE, COLOR_BLACK);

    // setup size
    int heightGap = (int)(LINES - row) / 2;
    int widthGap = (int)(COLS - col) / 2;

    // Create main window for title and borders
    mainWin = newwin(row, col, heightGap, widthGap);
    // Create subwindow with game
    win = derwin(mainWin, row - 2, col - 2, 1, 1);

    // Init game borders
    WindowSize.height = row - 2;
    WindowSize.width = col - 2;

    // Correct keypad
    keypad(win, TRUE);
    // Delay for drawing
    wtimeout(win, 100);
    // Init subwindow
    touchwin(win);
}

// Change flag for all loops
void close_game(__attribute__((unused)) int signum)
{
    exitGame = 1;
}
