#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <fcntl.h>
#include <signal.h>
#include <time.h>

#include "httpd.h"

#define CONNMAX 1000

char *method, // "GET" or "POST"
    *uri,     // "/index.html" things before '?'
    *qs,      // "a=1&b=2"     things after  '?'
    *prot;    // "HTTP/1.1"

char *payload; // for POST
int payload_size;
int request_size;
char *buf;

static int listenfd, clients[CONNMAX];
static void error(char *);
static void startServer(const char *);
static void respond(int);

typedef struct
{
    char *name, *value;
} header_t;
static header_t reqhdr[50] = {{"\0", "\0"}};
static int clientfd;

void serve_forever(const char *PORT)
{
    struct sockaddr_in clientaddr;
    socklen_t addrlen;
    char c;

    int slot = 0;

    printf(
        "HTTPD server started %shttp://0.0.0.0:%s%s\n",
        "\033[92m", PORT, "\033[0m");

    // Setting all elements to -1: signifies there is no client connected
    int i;
    for (i = 0; i < CONNMAX; i++)
        clients[i] = -1;
    startServer(PORT);

    // Ignore SIGCHLD to avoid zombie threads
    signal(SIGCHLD, SIG_IGN);

    // ACCEPT connections
    while (1)
    {
        addrlen = sizeof(clientaddr);
        clients[slot] = accept(listenfd, (struct sockaddr *)&clientaddr, &addrlen);

        if (clients[slot] < 0)
        {
            perror("accept() error");
        }
        else
        {
            if (fork() == 0)
            {
                respond(slot);
                exit(0);
            }
        }

        while (clients[slot] != -1)
            slot = (slot + 1) % CONNMAX;
    }
}

// start server
void startServer(const char *port)
{
    struct addrinfo hints, *res, *p;

    // getaddrinfo for host
    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE;
    if (getaddrinfo(NULL, port, &hints, &res) != 0)
    {
        perror("getaddrinfo() error");
        exit(1);
    }
    // socket and bind
    for (p = res; p != NULL; p = p->ai_next)
    {
        int option = 1;
        listenfd = socket(p->ai_family, p->ai_socktype, 0);
        setsockopt(listenfd, SOL_SOCKET, SO_REUSEADDR, &option, sizeof(option));
        if (listenfd == -1)
            continue;
        if (bind(listenfd, p->ai_addr, p->ai_addrlen) == 0)
            break;
    }
    if (p == NULL)
    {
        perror("socket() or bind()");
        exit(1);
    }

    freeaddrinfo(res);

    // listen for incoming connections
    if (listen(listenfd, 1000000) != 0)
    {
        perror("listen() error");
        exit(1);
    }
}

// get request header
char *request_header(const char *name)
{
    header_t *h = reqhdr;
    while (h->name)
    {
        if (strcmp(h->name, name) == 0)
            return h->value;
        h++;
    }
    return NULL;
}

void renderHtml(char *file)
{
    char html[10000];
    int fd = open(file, O_RDONLY);
    if (fd < 0)
    {
        printf(
            "HTTP/1.1 404 Not Found\r\n"
            "Server: Low Budget HTTPD v1.0\r\n"
            "Content-Type: text/html\r\n"
            "Content-Length: 9\r\n"
            "\r\n"
            "Not Found");
        return;
    }
    int n = read(fd, html, 10000);
    html[n] = 0;
    close(fd);

    printf(
        "HTTP/1.1 200 OK\r\n"
        "Server: Low Budget HTTPD v1.0\r\n"
        "Content-Type: text/html\r\n"
        "Content-Length: %d\r\n"
        "\r\n"
        "%s",
        n, html);
}

void redirect(char *location)
{
    printf(
        "HTTP/1.1 302 Found\r\n"
        "Server: Low Budget HTTPD v1.0\r\n"
        "Location: %s\r\n"
        "\r\n",
        location);
}

void serverError()
{
    printf(
        "HTTP/1.1 500 Internal Server Error\r\n"
        "Server: Low Budget HTTPD v1.0\r\n"
        "Content-Type: text/html; charset=UTF-8\r\n"
        "Content-Length: %d\r\n"
        "\r\n"
        "Internal Server Error",
        21);
}

void setLogin(char state)
{
    FILE *fp = fopen("/tmp/logged_in", "wb");
    fputc(state, fp);
    fclose(fp);
}

char getLogin()
{
    char state;
    FILE *fp = fopen("/tmp/logged_in", "rb");
    state = fgetc(fp);
    fclose(fp);
    return state;
}

// client connection
void respond(int n)
{
    int rcvd, fd, bytes_read;
    char *ptr;

    buf = malloc(65535);
    rcvd = recv(clients[n], buf, 65535, 0);

    if (rcvd < 0) // receive error
        fprintf(stderr, ("recv() error\n"));
    else if (rcvd == 0) // receive socket closed
        fprintf(stderr, "Client disconnected upexpectedly.\n");
    else // message received
    {
        buf[rcvd] = '\0';

        method = strtok(buf, " \t\r\n");
        uri = strtok(NULL, " \t");
        prot = strtok(NULL, " \t\r\n");

        fprintf(stderr, "[%s] %s\n", method, uri);

        if (qs = strchr(uri, '?'))
        {
            *qs++ = '\0'; // split URI
        }
        else
        {
            qs = uri - 1; // use an empty string
        }

        header_t *h = reqhdr;
        char *t, *t2;
        while (h < reqhdr + 50)
        {
            char *k, *v;
            k = strtok(NULL, "\r\n: \t");
            if (!k)
                break;
            v = strtok(NULL, "\r\n");
            while (*v && *v == ' ')
                v++;
            h->name = k;
            h->value = v;
            h++;
            // fprintf(stderr, "[H] %s: %s\n", k, v);
            t = v + 1 + strlen(v);
            if (t[1] == '\r' && t[2] == '\n')
                break;
        }
        payload = t + 3;                       // now the *t shall be the beginning of user payload
        t2 = request_header("Content-Length"); // and the related header if there is
        payload_size = t2 ? atol(t2) : (rcvd - (t - buf));
        request_size = rcvd;

        // bind clientfd to stdout, making it easier to write
        clientfd = clients[n];
        dup2(clientfd, STDOUT_FILENO);
        close(clientfd);

        // call router
        route();

        // tidy up
        fflush(stdout);
        free(buf);
        shutdown(STDOUT_FILENO, SHUT_WR);
        close(STDOUT_FILENO);
    }
    // Closing SOCKET
    shutdown(clientfd, SHUT_RDWR); // All further send and recieve operations are DISABLED...
    close(clientfd);
    clients[n] = -1;
}

void readFlag()
{
    if (getLogin())
        renderHtml("/home/httpd/flag.txt");
    else
        redirect("/login");
    // clean up for an easy ret2win exploit without the need for rop
    fflush(stdout);
    free(buf);
    shutdown(STDOUT_FILENO, SHUT_WR);
    close(STDOUT_FILENO);
    exit(0);
}