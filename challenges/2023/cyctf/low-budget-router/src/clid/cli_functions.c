#include "cli_functions.h"

int doAuthentication(int client_socket, char *user, char *pass, int trials)
{
    if (!strcmp(user, "admin") && !strcmp(pass, "notguessablebutnotcomplex"))
    {
        sendBytes(client_socket, "Authentication Successful.\n");
        return 1;
    }
    if (trials)
    {
        char msg[100];
        snprintf(msg, 100, "Failed to login, only %d trials left!\n", trials);
        sendBytes(client_socket, msg);
    }
    else
    {
        sendBytes(client_socket, "FAILED! Terminating session.\n");
    }
    return 0;
}

void getStatus(int client_socket)
{
    sendBytes(client_socket, "Router status: UP.\n");
}

void pwd(int client_socket)
{
    char *cwd = getcwd(NULL, 200);
    sendBytes(client_socket, cwd);
    sendNewline(client_socket);
    free(cwd);
}

void ls(int client_socket, char *arguments)
{
    char *path;
    int allocated = 0;

    if (arguments && strlen(arguments) != 0)
    {
        path = arguments;
    }
    else
    {
        path = getcwd(NULL, 200);
        allocated = 1;
    }

    if (!checkAbsPathBase(path, "/mnt/jail"))
    {
        sendBytes(client_socket, "Don't try to escape the jail!\n");
        return;
    }

    char *output = execCmd("ls -la %s", path, NULL);
    if (output)
    {
        sendBytes(client_socket, output);
        sendNewline(client_socket);
        free(output);
    }
    else
    {
        sendBytes(client_socket, "An error occurred!\n");
    }
}

void cd(int client_socket, char *arguments)
{
    char *original_directory = getcwd(NULL, 0);

    if (!arguments)
    {
        sendBytes(client_socket, "No directory provided!\n");
        return;
    }

    if (!checkAbsPathBase(arguments, "/mnt/jail"))
    {
        sendBytes(client_socket, "Don't try to escape the jail!\n");
        return;
    }

    if (chdir(arguments) == 0)
    {
        char *new_directory = getcwd(NULL, 0);

        if (new_directory != NULL)
        {
            sendBytes(client_socket, "Changed directory to: ");
            sendBytes(client_socket, new_directory);
            sendNewline(client_socket);
            free(new_directory);
        }
        else
        {
            sendBytes(client_socket, "Directory change was successful, but unable to retrieve new directory path.\n");
        }
    }
    else
    {
        perror("chdir");
        sendBytes(client_socket, "Failed to change directory.\n");
    }

    free(original_directory);
}

void ping(int client_socket, char *arguments)
{
    if (!diag)
    {
        sendBytes(client_socket, "Function is not implemented yet!\n");
    }
    else
    {
        if (!arguments)
        {
            sendBytes(client_socket, "No host provided!\n");
            return;
        }
        char *output = execCmd("ping -c3 %s | tee /tmp/ping_log", arguments, NULL);
        if (output)
        {
            sendBytes(client_socket, output);
            sendNewline(client_socket);
            free(output);
        }
        else
        {
            sendBytes(client_socket, "An error occurred!\n");
        }
    }
}

void readFlag(int client_socket)
{
    char *msg = "IMPOSTER! A real admin won't get the flag this way\nYou only get this: https://www.youtube.com/watch?v=dQw4w9WgXcQ\n";
    sendBytes(client_socket, msg);
}

void setDiag(int client_socket, char *arguments)
{
    if (!arguments)
    {
        sendBytes(client_socket, "Unknown command!\n");
        return;
    }
    unsigned int n = atoi(arguments);
    if (n == 0 || n == 1)
        diag = n;
    else
        sendBytes(client_socket, "Unknown command!\n");
}

void head(int client_socket, char *arguments)
{
    if (!arguments)
    {
        sendBytes(client_socket, "Unknown command!\n");
        return;
    }
    char *count = strtok(arguments, ",");
    char *filename = strtok(NULL, ",");
    if (!count || !filename)
    {
        sendBytes(client_socket, "Unknown command!\n");
        return;
    }
    // printf("filename: %s\ncount: '%.2s'\n", filename, count);
    char *output = execCmd("find /tmp/ -user 1002 -type f -path %s -exec head -n%.2s \"{}\" \\;", filename, count, NULL);
    if (output)
    {
        // printf("out: %s", output);
        sendBytes(client_socket, output);
        sendNewline(client_socket);
        free(output);
    }
    else
    {
        sendBytes(client_socket, "An error occurred!\n");
    }
}
