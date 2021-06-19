#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

#include <sys/select.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <netinet/in.h>
#include <net/if.h>
#include <arpa/inet.h>

#include "server.h"

void *udp_server(__attribute__((unused)) void *none)
{
    int sock, ret;
    unsigned int addr_len;
    fd_set readfd;
    struct sockaddr_in addr;

    sock = socket(AF_INET, SOCK_DGRAM, 0);
    if (sock < 0)
    {
        perror("socket");
        return NULL;
    }

// Нужно для работы моей впн сети
#ifdef VPN_NETWORK
    char *devname = "tap0";
    ret = setsockopt(sock, SOL_SOCKET, SO_BINDTODEVICE, devname, strlen(devname));
    if (ret == -1)
    {
        perror("setsockopt");
        return 0;
    }
#endif

    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = htonl(INADDR_ANY);
    addr.sin_port = htons(PORT);
    addr_len = sizeof(addr);

    if (bind(sock, (struct sockaddr *)&addr, addr_len) < 0)
    {
        perror("bind");
        close(sock);
        return NULL;
    }

    GameState state;
    while (!exitGame)
    {
        FD_ZERO(&readfd);
        FD_SET(sock, &readfd);

        ret = select(sock + 1, &readfd, NULL, NULL, 0);
        if (ret > 0)
        {
            if (FD_ISSET(sock, &readfd))
            {
                // recvfrom(sock, &key, sizeof(key), 0, (struct sockaddr *)&addr, &addr_len);
                recvfrom(sock, &state, sizeof(GameState), 0, (struct sockaddr *)&addr, &addr_len);
                for (int i = 0; i < 2; i++)
                {
                    if (IPs[i].sin_addr.s_addr == addr.sin_addr.s_addr)
                    {
                        if (*(int *)state.packetName == 0x0A)
                            update_status(i);
                        else
                            move_player(*(int *)state.packetName, i);
                    }
                }
                // Game Data can send only first player
                if (!PlayerNumber)
                    if (!strncmp(state.packetName, "STAT", 4))
                        send_game_state();

                if (!strncmp(state.packetName, "GAME", 4))
                    update_game_state(state);
                
                if(*(unsigned int *)state.packetName == 0xdeadbeef)
                    exitGame = 1;

                memset((void *)&state, 0, sizeof(GameState));
            }
        }
    }

    close(sock);
    return (void *)NULL;
}

void udp_client(int *sock, struct sockaddr_in *addr, int *addr_len)
{
    int yes = 1;
    int ret;

    *sock = socket(AF_INET, SOCK_DGRAM, 0);
    if (*sock < 0)
    {
        perror("sock");
        exit(1);
    }

    ret = setsockopt(*sock, SOL_SOCKET, SO_BROADCAST | SO_REUSEADDR, (char *)&yes, sizeof(yes));
    if (ret == -1)
    {
        perror("setsockopt");
        exit(1);
    }

// Нужно для работы моей впн сети
#ifdef VPN_NETWORK
    char *devname = "tap0";
    ret = setsockopt(*sock, SOL_SOCKET, SO_BINDTODEVICE, devname, strlen(devname));
    if (ret == -1)
    {
        perror("setsockopt");
        exit(1);
    }
#endif

    *addr_len = sizeof(struct sockaddr_in);
    memset((void *)addr, 0, *addr_len);
    addr->sin_family = AF_INET;
    addr->sin_addr.s_addr = htonl(INADDR_BROADCAST);
    addr->sin_port = htons(PORT);
}

void detect_ip(void){
    // Parse ip addr for spectator
    int fd;
    struct ifreq ifr;
    fd = socket(AF_INET, SOCK_DGRAM, 0);

    ifr.ifr_addr.sa_family = AF_INET;

#ifdef VPN_NETWORK
    strncpy(ifr.ifr_name, "tap0", IFNAMSIZ - 1);
#else
    strncpy(ifr.ifr_name, "eno1", IFNAMSIZ - 1);
#endif

    ioctl(fd, SIOCGIFADDR, &ifr);

    // Check spectator or not
    for (int i = 0; i < MAX_PLAYERS; i++)
    {
        if (IPs[i].sin_addr.s_addr == ((struct sockaddr_in *)&ifr.ifr_addr)->sin_addr.s_addr)
        {
            PlayerNumber = i;
            spectator = 0;
        }
    }

    close(fd);
}
