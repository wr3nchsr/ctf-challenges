#include "cli_helpers.h"

int sendBytes(int socket, const char *data)
{
    int sent_bytes = send(socket, data, strlen(data), 0);
    if (sent_bytes == -1)
    {
        perror("Send failed");
    }
    return sent_bytes;
}

int recvBytes(int socket, char *buffer, size_t buffer_size)
{
    int received_bytes = recv(socket, buffer, buffer_size - 1, 0);
    if (received_bytes == 0)
    {
        // Client disconnected gracefully
        printf("Client disconnected.\n");
        // You may choose to close the socket or take other actions
    }
    else if (received_bytes == -1)
    {
        perror("Receive failed");
        // Handle other errors or close the connection
    }
    else
    {
        // Strip newline character (if present) and null-terminate the received data
        buffer[received_bytes] = '\0'; // Null-terminate the string
        if (received_bytes > 0 && buffer[received_bytes - 1] == '\n')
        {
            // Remove newline character (if present)
            buffer[received_bytes - 1] = '\0';
        }
    }
    return received_bytes;
}

void sendNewline(int client_socket)
{
    sendBytes(client_socket, "\n");
}

char *readFile(const char *filename)
{
    FILE *file = fopen(filename, "rb");
    if (!file)
    {
        perror("Error opening the file");
        return NULL;
    }
    fseek(file, 0, SEEK_END);
    long file_size = ftell(file);
    fseek(file, 0, SEEK_SET);
    if (file_size <= 0)
    {
        fclose(file);
        return NULL;
    }
    char *file_content = (char *)malloc(file_size + 1);
    if (!file_content)
    {
        fclose(file);
        perror("Error allocating memory");
        return NULL;
    }
    size_t bytes_read = fread(file_content, 1, file_size, file);
    if (bytes_read != (size_t)file_size)
    {
        fclose(file);
        free(file_content);
        perror("Error reading the file");
        return NULL;
    }
    file_content[file_size] = '\0';
    fclose(file);
    return file_content;
}

int writeFile(const char *filename, const char *data, size_t data_length)
{
    FILE *file = fopen(filename, "wb");
    if (file == NULL)
    {
        perror("Error opening file");
        return 1;
    }
    size_t bytes_written = fwrite(data, 1, data_length, file);
    if (bytes_written != data_length)
    {
        perror("Error writing to file");
        fclose(file);
        return 1;
    }
    fclose(file);
    return 0;
}

char *execCmd(const char *baseCmdFormat, ...)
{
    va_list args;
    va_start(args, baseCmdFormat);

    char *userArgs;
    while ((userArgs = va_arg(args, char *)) != NULL)
    {
        if (strpbrk(userArgs, "&|;<>$`"))
        {
            perror("Trying to inject I see.\n");
            va_end(args);
            return NULL;
        }
    }
    va_end(args);

    va_start(args, baseCmdFormat);
    size_t cmdLength = vsnprintf(NULL, 0, baseCmdFormat, args);
    va_end(args);

    char *fullCmd = (char *)malloc(cmdLength + 1);

    if (!fullCmd)
    {
        perror("Memory allocation failed");
        return NULL;
    }

    va_start(args, baseCmdFormat);
    vsnprintf(fullCmd, cmdLength + 1, baseCmdFormat, args);
    va_end(args);
    // printf("CMD: %s\n", fullCmd);

    FILE *fp = popen(fullCmd, "r");
    if (fp == NULL)
    {
        perror("Command execution failed");
        free(fullCmd);
        return NULL;
    }

    char buffer[128];
    char *output = NULL;
    size_t outputSize = 0;

    while (fgets(buffer, sizeof(buffer), fp) != NULL)
    {
        size_t bufferLen = strlen(buffer);
        char *newOutput = (char *)realloc(output, outputSize + bufferLen + 1);

        if (!newOutput)
        {
            perror("Memory allocation failed");
            free(fullCmd);
            free(output);
            pclose(fp);
            return NULL;
        }

        output = newOutput;
        strcpy(output + outputSize, buffer);
        outputSize += bufferLen;
    }
    if (output)
        output[outputSize] = '\0';

    int result = pclose(fp);
    free(fullCmd);

    if (result == -1)
    {
        perror("Command execution failed");
        if (output)
            free(output);
        return NULL;
    }
    else
    {
        printf("Command executed successfully.\n");
        return output;
    }
}

int checkAbsPathBase(const char *path, const char *base)
{
    char *absolutePath;
    int out = 0;
    absolutePath = realpath(path, NULL);
    if (absolutePath && !strncmp(absolutePath, base, strlen(base)))
        out = 1;
    free(absolutePath);
    return out;
}