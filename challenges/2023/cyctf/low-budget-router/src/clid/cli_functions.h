#include <stdio.h>
#include <sys/types.h>
#include <dirent.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>

static unsigned int diag = 0;

int doAuthentication(int client_socket, char *user, char *pass, int trials);
void getStatus(int client_socket);
void pwd(int client_socket);
void ls(int client_socket, char *arguments);
void cd(int client_socket, char *arguments);
void ping(int client_socket, char *arguments);
void readFlag(int client_socket);
void setDiag(int client_socket, char *arguments);
void head(int client_socket, char *arguments);