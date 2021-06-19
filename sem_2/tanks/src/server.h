#ifndef __SERVER_H__
#define __SERVER_H__

#include <pthread.h>
#include <ncurses.h>
#include <signal.h>
#include "logic.h"

extern int PORT;
extern struct sockaddr_in IPs[2];
extern pthread_mutex_t mutex;
extern volatile sig_atomic_t exitGame;
extern void move_player(int key, int PlayerID);;
extern int update_status(int PlayerID);
extern void send_game_state(void);
extern void update_game_state(GameState state);
extern int PlayerNumber;
extern int spectator;

// Functions prototype
void* udp_server(void* none);
void udp_client(int* sock, struct sockaddr_in *addr, int* addr_len);
void detect_ip(void);

#endif