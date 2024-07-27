#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <pthread.h>
#include <signal.h>

#define PORT 10023
#define MAX_CLIENTS 10

#define THREAD_POOL_SIZE 5

typedef struct
{
    pthread_t threads[THREAD_POOL_SIZE];
    int available[THREAD_POOL_SIZE];
    pthread_mutex_t mutex;
} ThreadPool;

ThreadPool threadPool;

int authenticate(int client_socket)
{
    char username[30];
    char password[30];
    int received_bytes;
    int trials = 3;
    sendBytes(client_socket, "[ Login ]\n");
    do
    {
        if (!trials)
            return 0;

        sendBytes(client_socket, "Username: ");
        received_bytes = recvBytes(client_socket, username, sizeof(username));
        if (received_bytes == 0)
        {
            return 0;
        }
        sendBytes(client_socket, "Password: ");
        received_bytes = recvBytes(client_socket, password, sizeof(password));
        if (received_bytes == 0)
        {
            return 0;
        }
        trials--;
    } while (!doAuthentication(client_socket, username, password, trials));
    return 1;
}

void banner(int client_socket)
{
    sendBytes(client_socket, ""
                             "------------------------------------------\n"
                             "[      Low Budget Router - CLI v1.0      ]\n"
                             "------------------------------------------\n");
}

void help(int client_socket)
{
    sendBytes(client_socket, ""
                             "Low Budget CLI v1.0\n\n"
                             "help - print the available commands.\n"
                             "getstatus - get router status.\n"
                             "pwd - print the current directory.\n"
                             "ls - list the current directory.\n"
                             "cd - change the current directory.\n"
                             "ping - diagnose network with ping.\n"
                             "readflag - read the flag of this challenge.\n"
                             "exit - terminate connection.\n");
}

void interactiveShell(int client_socket)
{
    char input[100];
    char *cmd;
    char *arguments;
    while (1)
    {
        sendBytes(client_socket, "> ", 2);
        int received_bytes = recvBytes(client_socket, input, sizeof(input));
        if (received_bytes == 0 || received_bytes < 0)
        {
            break;
        }
        else if (received_bytes > 0)
        {
            cmd = input;
            char *space_position = strstr(input, " ");
            if (space_position)
            {
                *space_position = '\0';
                arguments = space_position + 1;
            }
            else
            {
                arguments = NULL;
            }

            // printf("CMD: %s, ARG: %s\n", cmd, arguments);
            if (strcasecmp(cmd, "help") == 0)
            {
                help(client_socket);
            }
            else if (strcasecmp(cmd, "getstatus") == 0)
            {
                getStatus(client_socket);
            }
            else if (strcasecmp(cmd, "pwd") == 0)
            {
                pwd(client_socket);
            }
            else if (strcasecmp(cmd, "ls") == 0)
            {
                ls(client_socket, arguments);
            }
            else if (strcasecmp(cmd, "cd") == 0)
            {
                cd(client_socket, arguments);
            }
            else if (strcasecmp(cmd, "ping") == 0)
            {
                ping(client_socket, arguments);
            }
            else if (strcasecmp(cmd, "readflag") == 0)
            {
                readFlag(client_socket);
            }
            else if (strcmp(cmd, "setdiag") == 0)
            {
                setDiag(client_socket, arguments);
            }
            else if (strcmp(cmd, "head") == 0)
            {
                head(client_socket, arguments);
            }
            else if (strcasecmp(cmd, "exit") == 0)
            {
                return;
            }
            else
            {
                if (sendBytes(client_socket, "Unknown command!\n") < 0)
                    return;
            }
        }
    }
}

void cleanup()
{
    system("rm /tmp/ping_log");
}

void threadSigsegvHandler(int signum)
{
    printf("Thread %lu: Segmentation fault occurred. Terminating the thread...\n", pthread_self());
    pthread_exit(NULL);
}

void *handleClient(void *socket_desc)
{
    signal(SIGPIPE, SIG_IGN);
    signal(SIGSEGV, threadSigsegvHandler);

    int client_socket = *(int *)socket_desc;
    if (authenticate(client_socket))
    {
        banner(client_socket);
        interactiveShell(client_socket);
        cleanup();
    }
    close(client_socket);

    signal(SIGSEGV, SIG_DFL);
    // Mark the thread as available in the thread pool
    pthread_mutex_lock(&threadPool.mutex);
    for (int i = 0; i < THREAD_POOL_SIZE; i++)
    {
        if (threadPool.threads[i] == pthread_self())
        {
            threadPool.available[i] = 1;
            break;
        }
    }
    pthread_mutex_unlock(&threadPool.mutex);
    free(socket_desc);
    return NULL;
}

int findAvailableThread()
{
    int index = -1;
    pthread_mutex_lock(&threadPool.mutex);
    for (int i = 0; i < THREAD_POOL_SIZE; i++)
    {
        if (threadPool.available[i])
        {
            index = i;
            threadPool.available[i] = 0;
            break;
        }
    }
    pthread_mutex_unlock(&threadPool.mutex);
    return index;
}

int server_socket;

void sigintHandler(int signum)
{
    printf("Got signal: %d!\nClosing the server...\n", signum);
    close(server_socket);
    exit(0);
}

int main()
{
    struct sockaddr_in server_addr, client_addr;
    int new_socket;
    socklen_t addr_len;

    // Initialize the thread pool
    for (int i = 0; i < THREAD_POOL_SIZE; i++)
    {
        threadPool.available[i] = 1;
    }
    pthread_mutex_init(&threadPool.mutex, NULL);

    // Create socket
    server_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (server_socket == -1)
    {
        perror("Could not create server socket");
        return 1;
    }

    // Prepare the sockaddr_in structure
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    server_addr.sin_port = htons(PORT);

    // Bind
    if (bind(server_socket, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0)
    {
        perror("Bind failed");
        return 1;
    }

    signal(SIGINT, sigintHandler);

    // Listen
    listen(server_socket, MAX_CLIENTS);
    printf(
        "CLID server started %s0.0.0.0:%d%s\n",
        "\033[92m", PORT, "\033[0m");
    // printf("Server listening on port %d...\n", PORT);
    chdir("/mnt/jail");

    while (1)
    {
        addr_len = sizeof(struct sockaddr_in);
        new_socket = accept(server_socket, (struct sockaddr *)&client_addr, &addr_len);
        if (new_socket < 0)
        {
            perror("Accept failed");
            return 1;
        }

        // Find an available thread in the pool and assign the new_socket to it
        int index = findAvailableThread();
        if (index == -1)
        {
            close(new_socket);
        }
        else
        {
            int *new_sock = malloc(1);
            *new_sock = new_socket;
            if (pthread_create(&threadPool.threads[index], NULL, handleClient, (void *)new_sock) < 0)
            {
                perror("Could not create thread");
                free(new_sock);
            }
        }
    }

    close(server_socket);
    return 0;
}
