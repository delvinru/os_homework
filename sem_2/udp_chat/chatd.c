#include <pthread.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/select.h>
#include <string.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <ncurses.h>

#ifndef PORT
#define PORT 9999
#endif

#define NAME_SIZE 32
#define BUFFER_SIZE 256
#define IP_SIZE 16

static _Atomic unsigned short work_flag = 1;
static _Atomic unsigned short firstRun = 1;

struct NamePacket
{
  char request[4];
  char name[NAME_SIZE];
};

struct MessagePacket
{
  char request[4];
  char data[BUFFER_SIZE];
};

struct Node
{
  char name[NAME_SIZE];
  char ip[IP_SIZE];
  struct Node *next;
  struct Node *prev;
};

struct Node *head; // global variable - pointer to head node.

//Creates a new Node and returns pointer to it.
struct Node *GetNewNode(const char *name, const char *ip)
{
  struct Node *newNode = (struct Node *)calloc(1, sizeof(struct Node));
  memcpy(newNode->name, name, NAME_SIZE);
  memcpy(newNode->ip, ip, IP_SIZE);
  newNode->prev = NULL;
  newNode->next = NULL;
  return newNode;
}

//Inserts a Node at head of doubly linked list
void InsertAtHead(const char *name, const char *ip)
{
  struct Node *newNode = GetNewNode(name, ip);
  if (head == NULL)
  {
    head = newNode;
    return;
  }
  head->prev = newNode;
  newNode->next = head;
  head = newNode;
}

// Search username in list with ip
void GetNameByIp(const char *ip, char *name)
{
  struct Node *tmp = head;
  if (NULL == tmp)
    return;

  while (NULL != tmp)
  {
    if (!strcmp(tmp->ip, ip))
    {
      memcpy(name, tmp->name, NAME_SIZE);
      return;
    }
    tmp = tmp->next;
  }
  return;
}

// Start udp server
void *udp_server(void *none)
{
  int sock, count, ret;
  unsigned int addr_len;
  fd_set readfd;
  struct sockaddr_in addr;
  struct NamePacket *nick = (struct NamePacket *)calloc(1, sizeof(struct NamePacket));
  struct MessagePacket *msg = (struct MessagePacket *)calloc(1, sizeof(struct MessagePacket));

  sock = socket(AF_INET, SOCK_DGRAM, 0);
  if (0 > sock)
  {
    perror("socket");
    return NULL;
  }

// Нужно для работы моей впн сети
#ifdef TEST_FLAG
  char *devname = "tap0";
  ret = setsockopt(sock, SOL_SOCKET, SO_BINDTODEVICE, devname, strlen(devname));
  if (ret == -1)
  {
    perror("setsockopt");
    return 0;
  }
#endif

  memset((void *)&addr, 0, addr_len);
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

  while (work_flag)
  {
    FD_ZERO(&readfd);
    FD_SET(sock, &readfd);

    ret = select(sock + 1, &readfd, NULL, NULL, 0);
    if (ret > 0)
    {
      if (!firstRun && FD_ISSET(sock, &readfd))
      {
        count = recvfrom(sock, msg, sizeof(*msg), 0, (struct sockaddr *)&addr, &addr_len);

        // Got Nick packet
        if (!strncmp(msg->request, "NICK", 4))
        {
          char *tmp = (char *)calloc(NAME_SIZE, sizeof(char));
          GetNameByIp(inet_ntoa(addr.sin_addr), tmp);
          if (tmp[0] != '\0')
          {
            free(tmp);
            tmp = NULL;
          }
          else
          {
            nick = (struct NamePacket *)msg;
            InsertAtHead(nick->name, inet_ntoa(addr.sin_addr));
            fprintf(stderr, "\rClient [%s] with ip %s joined to chat!\n", nick->name, inet_ntoa(addr.sin_addr));
          }
        }
        else if (!strncmp(msg->request, "MSG", 4))
        {
          char *tmp = (char *)calloc(NAME_SIZE, sizeof(char));

          GetNameByIp(inet_ntoa(addr.sin_addr), tmp);
          fprintf(stderr, "\r\033[1m[%s]\033[0m %s\n", tmp, msg->data);

          free(tmp);
          tmp = NULL;
        }
        fprintf(stderr, "\r>> ");
      }
    }
  }
  close(sock);

  return (void *)NULL;
}

void stop_flag(int signum)
{
  work_flag = 0;
}

int main(int argc, char *argv[])
{
  int sock, yes = 1;
  pthread_t pid;
  struct sockaddr_in addr;
  int addr_len;
  int ret;
  char buffer[BUFFER_SIZE];
  struct NamePacket *nick = (struct NamePacket *)calloc(1, sizeof(struct NamePacket));
  struct MessagePacket *msg = (struct MessagePacket *)calloc(1, sizeof(struct MessagePacket));

  // Setup signint
  signal(SIGINT, stop_flag);

  pthread_create(&pid, NULL, udp_server, NULL);

  sock = socket(AF_INET, SOCK_DGRAM, 0);
  if (sock < 0)
  {
    perror("sock");
    return -1;
  }

  ret = setsockopt(sock, SOL_SOCKET, SO_BROADCAST | SO_REUSEADDR, (char *)&yes, sizeof(yes));
  if (ret == -1)
  {
    perror("setsockopt");
    return 0;
  }

// Нужно для работы моей впн сети
#ifdef TEST_FLAG
  char *devname = "tap0";
  ret = setsockopt(sock, SOL_SOCKET, SO_BINDTODEVICE, devname, strlen(devname));
  if (ret == -1)
  {
    perror("setsockopt");
    return 0;
  }
#endif

  addr_len = sizeof(struct sockaddr_in);
  memset((void *)&addr, 0, addr_len);
  addr.sin_family = AF_INET;
  addr.sin_addr.s_addr = htonl(INADDR_BROADCAST);
  addr.sin_port = htons(PORT);

  while (work_flag)
  {
    memset(buffer, 0, BUFFER_SIZE);

    // Read username
    if (firstRun)
    {
      fprintf(stderr, "=== Welcome to the Nice Chat ===\n");
      int correct = 0;
      while (!correct)
      {
        fprintf(stderr, "Enter your name: ");
        fgets(buffer, NAME_SIZE, stdin);
        ret = strlen(buffer);
        if (buffer[ret - 1] == '\n')
          buffer[ret - 1] = 0;
        if (buffer[0] != 0)
          correct = 1;
        else
          fprintf(stderr, "Empty name not allowed!\n");
      }

      firstRun = 0;
      // Save user name in struct
      char *request = "NICK";
      memcpy(nick->request, request, 4);
      memcpy(nick->name, buffer, NAME_SIZE);
      sendto(sock, nick, sizeof(*nick), 0, (struct sockaddr *)&addr, addr_len);
    }
    else
    {
      fprintf(stderr, "\r>> ");
      fgets(buffer, 1024, stdin);
      ret = strlen(buffer);
      if (buffer[ret - 1] == '\n')
        buffer[ret - 1] = 0;

      // Just delete inputed line
      fprintf(stderr, "\033[A\r");

      char *request = "MSG";
      memcpy(msg->request, request, 4);
      memset(msg->data, 0, BUFFER_SIZE);
      memcpy(msg->data, buffer, BUFFER_SIZE);

      // Repeat message send
      sendto(sock, nick, sizeof(*nick), 0, (struct sockaddr *)&addr, addr_len);
      sendto(sock, msg, sizeof(*msg), 0, (struct sockaddr *)&addr, addr_len);
    }
  }

  free(nick);
  free(msg);
  nick = NULL;
  msg = NULL;

  pthread_join(pid, NULL);
  close(sock);
  return 0;
}
