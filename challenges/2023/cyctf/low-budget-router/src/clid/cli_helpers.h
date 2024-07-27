#include <stdio.h>
#include <stddef.h>
#include <string.h>
#include <stdlib.h>
#include <stdarg.h>

int sendBytes(int socket, const char *data);
int recvBytes(int socket, char *buffer, size_t buffer_size);
void sendNewline(int client_socket);
char *readFile(const char *filename);
int writeFile(const char *filename, const char *data, size_t data_length);
char *execCmd(const char *baseCmdFormat, ...);
int checkAbsPathBase(const char *path, const char *base);