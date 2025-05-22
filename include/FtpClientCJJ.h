#ifndef FTPCLIENTCJJ_H
#define FTPCLIENTCJJ_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdbool.h>
#include <sys/stat.h>
#include "StringCJJ.h"
#include <sys/stat.h>

#include <sys/select.h> //tentar corrigir o erro de nao conectar
#include <errno.h>

#define TIMEOUT 3

typedef struct
{
    int controlSocket;
    int dataSocket;
    StrCJJ *host;
    StrCJJ *ip;
    uint16_t port;

    StrCJJ *user;
    StrCJJ *password;

    StrCJJ *urlPath;
    StrCJJ *filename;

    StrCJJ *responseBuffer;
} FTPClient;

FTPClient *FTPClient_create(const char *ftpUrl);
static bool create_directories(const char *path);

int connect_with_timeout(int sockfd, struct sockaddr *addr, socklen_t addrlen, int timeout_sec); // sugestao do gemini para dar fix ao problema

bool parse_ip_part(const char *part_str, uint8_t *out);
bool parse_port_part(const char *part_str, uint16_t *out);

void FTPClient_destroy(FTPClient *client);
bool FTPClient_sendCommand(int socket, const char *command, FTPClient *client);
bool FTPClient_receiveResponse(int socket, FTPClient *client);
bool FTPClient_connect(FTPClient *client);
bool FTPClient_login(FTPClient *client);
bool FTPClient_enterPassiveMode(FTPClient *client);
bool FTPClient_downloadFile(FTPClient *client);
void FTPClient_disconnect(FTPClient *client);
bool FTPClient_list(FTPClient *client, const char *pathname);
bool FTPClient_checkForValidClient(FTPClient *client);

#endif