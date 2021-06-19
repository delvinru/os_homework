#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <ncurses.h>
#include <pthread.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <netinet/in.h>
#include <net/if.h>
#include <arpa/inet.h>

#include "logic.h"
#include "server.h"
#include "map.h"

// Game object
Game game;
// Win flag
int winFlag = 0;
// Game start status
int player1Ready = 0;
int player2Ready = 0;
// Player Number
int PlayerNumber = 0;

// Skip spectator window
int spectator = 1;
// Spectator counter
int SpectatorCount = 0;

// Init socket stuff
int sock;
struct sockaddr_in addr;
int addr_len;

void send_game_state(void)
{
    // If not spectator send current game state
    pthread_mutex_lock(&mutex);
    if (!spectator)
    {
        // Send game state
        GameState state;
        memset((void *)&state, 0, sizeof(GameState));
        memcpy(state.packetName, "GAME", 4);
        state.game = game;
        // Drop bullet state
        // It's fix bug with freezed bullet without updating thread
        for (int i = 0; i < MAX_PLAYERS; i++)
        {
            state.game.tanks[i].amount_bullets = 0;
            for (int j = 0; j < MAX_BULLETS; j++)
            {
                state.game.tanks[i].bullet[j].alive = false;
            }
        }
        sendto(sock, &state, sizeof(GameState), 0, (struct sockaddr *)&addr, addr_len);
    }
    pthread_mutex_unlock(&mutex);
}

void update_game_state(GameState state)
{
    // Update current game state for spectator
    if (spectator)
        game = state.game;
    // If accept request for update game state increase SpectatorCounter
    SpectatorCount++;
}

bool check_bullet_collision_with_map(int x, int y)
{
    if (gameMap[y * WindowSize.width + x] != 0x00)
        return true;
    return false;
}

bool check_player_collision_with_map(int x, int y, enum MoveDirection dir)
{
    switch (dir)
    {
    case UP:
        if (
            gameMap[(y - 2) * WindowSize.width + x] != 0x00 ||
            gameMap[(y - 2) * WindowSize.width + x - 1] != 0x00 ||
            gameMap[(y - 2) * WindowSize.width + x + 1] != 0x00)
            return true;
        break;
    case DOWN:
        if (
            gameMap[(y + 2) * WindowSize.width + x] != 0x00 ||
            gameMap[(y + 2) * WindowSize.width + x - 1] != 0x00 ||
            gameMap[(y + 2) * WindowSize.width + x + 1] != 0x00)
            return true;
        break;
    case LEFT:
        if (
            gameMap[(y - 1) * WindowSize.width + x - 2] != 0x00 ||
            gameMap[y * WindowSize.width + x - 2] != 0x00 ||
            gameMap[(y + 1) * WindowSize.width + x - 2] != 0x00)
            return true;
        break;
    case RIGHT:
        if (
            gameMap[(y - 1) * WindowSize.width + x + 2] != 0x00 ||
            gameMap[y * WindowSize.width + x + 2] != 0x00 ||
            gameMap[(y + 1) * WindowSize.width + x + 2] != 0x00)
            return true;
        break;

    default:
        return false;
    }
    return false;
}

// Check collision with other tank
bool check_collision(int x1, int y1, int x2, int y2, int x3, int y3, int x4, int y4)
{
    if ((x1 <= x4) && (x2 >= x3) && (y1 <= y4) && (y2 >= y3))
        return true;
    return false;
}

void move_player(int key, int PlayerID)
{
    // Lock thread for right shared object change
    pthread_mutex_lock(&mutex);

    // Thread for bullet
    pthread_t bullet;

    // Second player rectangle
    int x1, y1, x2, y2;
    int x3 = game.tanks[!PlayerID].pos.x - 1;
    int y3 = game.tanks[!PlayerID].pos.y - 1;
    int x4 = x3 + 2;
    int y4 = y3 + 2;

    // Check keys for move tank
    switch (key)
    {
    case KEY_UP:
        x1 = game.tanks[PlayerID].pos.x - 1;
        y1 = game.tanks[PlayerID].pos.y - 1 - 1;
        x2 = x1 + 2;
        y2 = y1 + 2;

        game.tanks[PlayerID].direction = UP;
        // Check border
        if (game.tanks[PlayerID].pos.y - 1 > 0 &&
            !check_collision(x1, y1, x2, y2, x3, y3, x4, y4) &&
            !check_player_collision_with_map(game.tanks[PlayerID].pos.x, game.tanks[PlayerID].pos.y, UP))
            game.tanks[PlayerID].pos.y--;
        break;
    case KEY_DOWN:
        x1 = game.tanks[PlayerID].pos.x - 1;
        y1 = game.tanks[PlayerID].pos.y + 1 - 1;
        x2 = x1 + 2;
        y2 = y1 + 2;

        game.tanks[PlayerID].direction = DOWN;
        // Check border
        if (game.tanks[PlayerID].pos.y + 1 <= WindowSize.height - 2 &&
            !check_collision(x1, y1, x2, y2, x3, y3, x4, y4) &&
            !check_player_collision_with_map(game.tanks[PlayerID].pos.x, game.tanks[PlayerID].pos.y, DOWN))
            game.tanks[PlayerID].pos.y++;
        break;
    case KEY_LEFT:
        x1 = game.tanks[PlayerID].pos.x - 1 - 1;
        y1 = game.tanks[PlayerID].pos.y - 1;
        x2 = x1 + 2;
        y2 = y1 + 2;

        game.tanks[PlayerID].direction = LEFT;
        // Check border
        if (game.tanks[PlayerID].pos.x - 1 > 0 &&
            !check_collision(x1, y1, x2, y2, x3, y3, x4, y4) &&
            !check_player_collision_with_map(game.tanks[PlayerID].pos.x, game.tanks[PlayerID].pos.y, LEFT))
            game.tanks[PlayerID].pos.x--;
        break;
    case KEY_RIGHT:
        x1 = game.tanks[PlayerID].pos.x + 1 - 1;
        y1 = game.tanks[PlayerID].pos.y - 1;
        x2 = x1 + 2;
        y2 = y1 + 2;

        game.tanks[PlayerID].direction = RIGHT;
        // Check border
        if (game.tanks[PlayerID].pos.x + 1 <= WindowSize.width - 2 &&
            !check_collision(x1, y1, x2, y2, x3, y3, x4, y4) &&
            !check_player_collision_with_map(game.tanks[PlayerID].pos.x, game.tanks[PlayerID].pos.y, RIGHT))
            game.tanks[PlayerID].pos.x++;
        break;
    case ' ':
        if (game.tanks[PlayerID].amount_bullets >= MAX_BULLETS)
            break;
        pthread_create(&bullet, NULL, spawn_bullet, (void *)&game.tanks[PlayerID]);
        break;
    case 'q':
        game.tanks[PlayerID].gun = UP_LEFT;
        break;
    case 'w':
        game.tanks[PlayerID].gun = UP_UP;
        break;
    case 'e':
        game.tanks[PlayerID].gun = UP_RIGHT;
        break;
    case 'a':
        game.tanks[PlayerID].gun = MIDDLE_LEFT;
        break;
    case 'd':
        game.tanks[PlayerID].gun = MIDDLE_RIGHT;
        break;
    case 'z':
        game.tanks[PlayerID].gun = DOWN_LEFT;
        break;
    case 'x':
        game.tanks[PlayerID].gun = DOWN_DOWN;
        break;
    case 'c':
        game.tanks[PlayerID].gun = DOWN_RIGHT;
        break;
    }
    pthread_mutex_unlock(&mutex);
}

int update_status(int PlayerID)
{
    if ((player1Ready && player2Ready) || spectator)
        return 0;
    pthread_mutex_lock(&mutex);
    if (PlayerID == 0)
        player1Ready = 1;
    else if (PlayerID == 1)
        player2Ready = 1;
    print_tank_borders(PlayerID, false);
    wrefresh(win);
    pthread_mutex_unlock(&mutex);
    return 0;
}

// Function return number
// 0 - bullet missed
// 1 - bullet hit player 1
// 2 - bullet hit player 2
int check_hit(int BulletID, int PlayerID)
{
    int x = game.tanks[PlayerID].bullet[BulletID].pos.x;
    int y = game.tanks[PlayerID].bullet[BulletID].pos.y;
    int x1 = game.tanks[!PlayerID].pos.x - 1;
    int y1 = game.tanks[!PlayerID].pos.y - 1;
    int x2 = x1 + 2;
    int y2 = y1 + 2;
    if (x >= x1 && x <= x2 && y >= y1 && y <= y2)
        return (PlayerID == 0 ? 2 : 1);

    return 0;
}

void *spawn_bullet(void *data)
{
    // Search gun direction and setup bullet vector
    Player *tank = (Player *)data;

    // Increase counter for bullet
    tank->amount_bullets++;

    // Init BulletID
    int i = 0;
    for (int j = 0; j < MAX_BULLETS; j++)
    {
        if (!tank->bullet[j].alive)
        {
            i = j;
            break;
        }
    }

    // Setup alive bullet
    tank->bullet[i].alive = true;

    switch (tank->gun)
    {
    case UP_LEFT:
        tank->bullet[i].direction.x = -1;
        tank->bullet[i].direction.y = -1;
        tank->bullet[i].pos.x = tank->pos.x - 1;
        tank->bullet[i].pos.y = tank->pos.y - 1;
        break;
    case UP_UP:
        tank->bullet[i].direction.x = 0;
        tank->bullet[i].direction.y = -1;
        tank->bullet[i].pos.x = tank->pos.x;
        tank->bullet[i].pos.y = tank->pos.y - 1;
        break;
    case UP_RIGHT:
        tank->bullet[i].direction.x = 1;
        tank->bullet[i].direction.y = -1;
        tank->bullet[i].pos.x = tank->pos.x + 1;
        tank->bullet[i].pos.y = tank->pos.y - 1;
        break;
    case MIDDLE_LEFT:
        tank->bullet[i].direction.x = -1;
        tank->bullet[i].direction.y = 0;
        tank->bullet[i].pos.x = tank->pos.x - 1;
        tank->bullet[i].pos.y = tank->pos.y;
        break;
    case MIDDLE_RIGHT:
        tank->bullet[i].direction.x = 1;
        tank->bullet[i].direction.y = 0;
        tank->bullet[i].pos.x = tank->pos.x + 1;
        tank->bullet[i].pos.y = tank->pos.y;
        break;
    case DOWN_LEFT:
        tank->bullet[i].direction.x = -1;
        tank->bullet[i].direction.y = 1;
        tank->bullet[i].pos.x = tank->pos.x - 1;
        tank->bullet[i].pos.y = tank->pos.y + 1;
        break;
    case DOWN_DOWN:
        tank->bullet[i].direction.x = 0;
        tank->bullet[i].direction.y = 1;
        tank->bullet[i].pos.x = tank->pos.x;
        tank->bullet[i].pos.y = tank->pos.y + 1;
        break;
    case DOWN_RIGHT:
        tank->bullet[i].direction.x = 1;
        tank->bullet[i].direction.y = 1;
        tank->bullet[i].pos.x = tank->pos.x + 1;
        tank->bullet[i].pos.y = tank->pos.y + 1;
        break;
    }

    // Hit result
    int res;
    while (
        tank->bullet[i].pos.x > 0 &&
        tank->bullet[i].pos.x < WindowSize.width - 1 &&
        tank->bullet[i].pos.y > 0 &&
        tank->bullet[i].pos.y < WindowSize.height - 1 &&
        !check_bullet_collision_with_map(tank->bullet[i].pos.x, tank->bullet[i].pos.y))
    {
        pthread_mutex_lock(&mutex);
        res = check_hit(i, tank->PlayerID);
        if (res)
        {
            winFlag = res;
            pthread_mutex_unlock(&mutex);
            break;
        }
        tank->bullet[i].pos.x += tank->bullet[i].direction.x;
        tank->bullet[i].pos.y += tank->bullet[i].direction.y;
        pthread_mutex_unlock(&mutex);
        usleep(BULLET_TIME);
    }
    pthread_mutex_lock(&mutex);
    tank->amount_bullets--;
    tank->bullet[i].alive = false;
    pthread_mutex_unlock(&mutex);

    return (void *)NULL;
}

void update_spectator_count(void)
{
    char amountSpectators[32];
    memset(amountSpectators, 0, 32);
    // Update spectators title
    wattron(mainWin, COLOR_PAIR(1));
    sprintf(amountSpectators, " === Spectators: %d === ", SpectatorCount);
    mvwprintw(mainWin, 39, (WindowSize.width - strlen(amountSpectators)) / 2, amountSpectators);
    wattroff(mainWin, COLOR_PAIR(1));
    wrefresh(mainWin);
}

void print_bullet(Player tank, int j)
{
    pthread_mutex_lock(&mutex);
    mvwaddch(win, tank.bullet[j].pos.y, tank.bullet[j].pos.x, TANK_BULLET);
    pthread_mutex_unlock(&mutex);
}

void print_bullets()
{
    for (int i = 0; i < MAX_PLAYERS; i++)
    {
        for (int j = 0; j < MAX_BULLETS; j++)
        {
            if (game.tanks[i].bullet[j].alive)
                print_bullet(game.tanks[i], j);
        }
    }
}

void print_tank_borders(int PlayerID, bool blink)
{
    blink ? wattron(win, WA_BLINK) : 0;
    PlayerID ? wattron(win, COLOR_PAIR(3)) : wattron(win, COLOR_PAIR(2));
    for (int i = 0; i < MAX_PLAYERS; i++)
    {
        for (int y = -3; y < 4; y++)
        {
            for (int x = -4; x < 5; x++)
            {
                if (y == -3 || y == 3)
                    mvwaddch(win, game.tanks[PlayerID].pos.y + y, game.tanks[PlayerID].pos.x + x, ACS_HLINE);
                if (x == -4 || x == 4)
                    mvwaddch(win, game.tanks[PlayerID].pos.y + y, game.tanks[PlayerID].pos.x + x, ACS_VLINE);
                if (x == -4 && y == -3)
                    mvwaddch(win, game.tanks[PlayerID].pos.y + y, game.tanks[PlayerID].pos.x + x, ACS_ULCORNER);
                else if (x == -4 && y == 3)
                    mvwaddch(win, game.tanks[PlayerID].pos.y + y, game.tanks[PlayerID].pos.x + x, ACS_LLCORNER);
                else if (x == 4 && y == -3)
                    mvwaddch(win, game.tanks[PlayerID].pos.y + y, game.tanks[PlayerID].pos.x + x, ACS_URCORNER);
                else if (x == 4 && y == 3)
                    mvwaddch(win, game.tanks[PlayerID].pos.y + y, game.tanks[PlayerID].pos.x + x, ACS_LRCORNER);
            }
        }
    }
    blink ? wattroff(win, WA_BLINK) : 0;
    PlayerID ? wattroff(win, COLOR_PAIR(3)) : wattroff(win, COLOR_PAIR(2));
}

void print_tank(Player tank)
{
    for (int x = -1; x <= 1; x++)
    {
        for (int y = -1; y <= 1; y++)
        {
            switch (tank.direction)
            {
            case UP:
            case DOWN:
                if ((x == 0 && y == -1) || (x == 0 && y == 1))
                    mvwaddch(win, tank.pos.y + y, tank.pos.x + x, ' ');
                else
                    mvwaddch(win, tank.pos.y + y, tank.pos.x + x, TANK_TRACKS);
                break;
            case LEFT:
            case RIGHT:
                if ((x == -1 && y == 0) || (x == 1 && y == 0))
                    mvwaddch(win, tank.pos.y + y, tank.pos.x + x, ' ');
                else
                    mvwaddch(win, tank.pos.y + y, tank.pos.x + x, TANK_TRACKS);
                break;
            }
        }
    }

    mvwaddch(win, tank.pos.y, tank.pos.x, TANK_CENTER);

    switch (tank.gun)
    {
    case UP_LEFT:
        mvwaddch(win, tank.pos.y - 1, tank.pos.x - 1, '\\');
        break;
    case UP_UP:
        mvwaddch(win, tank.pos.y - 1, tank.pos.x + 0, '|');
        break;
    case UP_RIGHT:
        mvwaddch(win, tank.pos.y - 1, tank.pos.x + 1, '/');
        break;
    case MIDDLE_LEFT:
        mvwaddch(win, tank.pos.y - 0, tank.pos.x - 1, '-');
        break;
    case MIDDLE_RIGHT:
        mvwaddch(win, tank.pos.y - 0, tank.pos.x + 1, '-');
        break;
    case DOWN_LEFT:
        mvwaddch(win, tank.pos.y + 1, tank.pos.x - 1, '/');
        break;
    case DOWN_DOWN:
        mvwaddch(win, tank.pos.y + 1, tank.pos.x - 0, '|');
        break;
    case DOWN_RIGHT:
        mvwaddch(win, tank.pos.y + 1, tank.pos.x + 1, '\\');
        break;
    }
}

void print_tanks(Player *tank)
{
    pthread_mutex_lock(&mutex);
    // Draw first player
    wattron(win, COLOR_PAIR(2));
    print_tank(tank[0]);
    wattroff(win, COLOR_PAIR(2));

    // Draw second player
    wattron(win, COLOR_PAIR(3));
    print_tank(tank[1]);
    wattroff(win, COLOR_PAIR(3));

    pthread_mutex_unlock(&mutex);
}

void print_map()
{
    for (int y = 0; y < WindowSize.height; y++)
    {
        for (int x = 0; x < WindowSize.width; x++)
        {
            if (gameMap[y * WindowSize.width + x] != 0x00)
                mvwaddch(win, y, x, '#');
        }
    }
}

Game init_game(void)
{
    Game game;
    for (int i = 0; i < MAX_PLAYERS; i++)
    {
        // By default tank move up
        game.tanks[i].direction = UP;
        // By default gun firection up
        game.tanks[i].gun = UP_UP;
        game.tanks[i].amount_bullets = 0;
        game.tanks[i].PlayerID = i;
        for (int j = 0; j < MAX_BULLETS; j++)
        {
            game.tanks[i].bullet[j].pos.x = -1;
            game.tanks[i].bullet[j].pos.y = -1;
            game.tanks[i].bullet[j].alive = false;
        }
    }

    // Init static players position
    game.tanks[0].pos.x = WindowSize.width / 4;
    game.tanks[0].pos.y = WindowSize.height / 2;
    game.tanks[1].pos.x = (WindowSize.width * 3 / 4);
    game.tanks[1].pos.y = WindowSize.height / 2;

    return game;
}

void *game_loop()
{
    udp_client(&sock, &addr, &addr_len);
    detect_ip();

    // Constants string
    char *title = " === Online Tank Battle === ";
    char *spectatorTitle = " === Spectator === ";
    char *startMsg = "Press ENTER to be ready for the game";
    char winMsg[32];
    char *exitMsg = "Press ENTER to exit...";

    // Input charachter
    int ch;
    // Frame rate. Like 120 FPS :)
    int frame = 1000000 / 120;

    // Just print box title
    box(mainWin, 0, 0);
    wattron(mainWin, COLOR_PAIR(1));
    mvwprintw(mainWin, 0, (WindowSize.width - strlen(title)) / 2, title);
    wattroff(mainWin, COLOR_PAIR(1));
    wrefresh(mainWin);

    // Init game object
    game = init_game();

    // Check if current player spectator skip game wait
    if (!spectator)
    {
        wtimeout(win, -1);
        // Draw init screen
        print_tanks(game.tanks);
        mvwprintw(win, WindowSize.height / 2, (WindowSize.width - strlen(startMsg)) / 2, startMsg);
        print_tank_borders(0, true);
        print_tank_borders(1, true);
        wrefresh(win);
        update_spectator_count();

        ch = wgetch(win);
        ch = 0x0A;
        while (!exitGame)
        {
            if (player1Ready && player2Ready)
                break;
            update_spectator_count();
            sendto(sock, &ch, sizeof(ch), 0, (struct sockaddr *)&addr, addr_len);
            usleep(frame * 60);
        }

        // Restore timeout
        wtimeout(win, 100);
    }
    else
    {
        wattron(mainWin, COLOR_PAIR(1));
        mvwprintw(mainWin, 39, (WindowSize.width - strlen(spectatorTitle)) / 2, spectatorTitle);
        wattroff(mainWin, COLOR_PAIR(1));
        wrefresh(mainWin);

        // Send request stat
        char *req = "STAT";
        sendto(sock, req, sizeof(*req) * 4, 0, (struct sockaddr *)&addr, addr_len);
    }

    // Main game loop
    while (!exitGame && !winFlag)
    {
        // Clean window
        werase(win);

        // Print stuff
        print_tanks(game.tanks);
        print_bullets();
        print_map();
        wrefresh(win);

        // Grab input from user and send to the server
        ch = wgetch(win);
        if (ch != ERR)
            sendto(sock, &ch, sizeof(ch), 0, (struct sockaddr *)&addr, addr_len);

        if (!spectator)
            update_spectator_count();

        // Frames update
        usleep(frame);
    }

    // Sleep for cooldown
    usleep(100000);
    // Print final message
    // Clean window
    werase(win);
    if (winFlag == 1)
        sprintf(winMsg, "Player 2 WIN in this battle");
    else if (winFlag == 2)
        sprintf(winMsg, "Player 1 WIN in this battle");
    else
        sprintf(winMsg, "Game closed...");

    winFlag == 1 ? wattron(win, COLOR_PAIR(3)) : wattron(win, COLOR_PAIR(2));
    mvwprintw(win, WindowSize.height / 2, (WindowSize.width - strlen(winMsg)) / 2, winMsg);
    winFlag == 1 ? wattroff(win, COLOR_PAIR(3)) : wattroff(win, COLOR_PAIR(2));

    mvwprintw(win, WindowSize.height - 2, (WindowSize.width - strlen(exitMsg)) / 2, exitMsg);
    exitGame = 1;

    // Send to the network end stuff
    if (!spectator)
    {
        ch = 0xdeadbeef;
        sendto(sock, &ch, sizeof(ch), 0, (struct sockaddr *)&addr, addr_len);
    }

    // Close opened socket
    close(sock);

    // Wait for last key
    wtimeout(win, -1);
    while ((ch = wgetch(win)) != 0x0a)
        ;

    // Stop udp server thread
    pthread_cancel(tid2);

    return (void *)NULL;
}
