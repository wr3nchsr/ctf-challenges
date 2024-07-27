#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <signal.h>

#define MAX_DAEMONS 2

struct daemon
{
    uid_t uid;
    gid_t gid;
    char binary[20];
    pid_t pid;
} child_daemons[MAX_DAEMONS];

void sigterm_handler(int signum)
{
    puts("Received SIGTERM. Terminating daemons...");
    for (int i = 0; i < MAX_DAEMONS; i++)
    {
        if (child_daemons[i].pid > 0)
        {
            kill(child_daemons[i].pid, SIGTERM);
        }
    }
    exit(0);
}

void runDaemon(uid_t user_uid, gid_t user_gid, const char *binary_path, int daemon_index)
{
    pid_t child_pid;
    child_pid = fork();

    if (child_pid == -1)
    {
        perror("fork");
        exit(1);
    }
    else if (child_pid == 0)
    {
        if (setgid(user_gid) == -1 || setuid(user_uid) == -1)
        {
            perror("setgid/setuid");
            exit(1);
        }

        if (execl(binary_path, (char *)NULL) == -1)
        {
            perror("execl");
            exit(1);
        }
    }
    else
    {
        child_daemons[daemon_index].pid = child_pid;
        printf("%s running as %d\n", child_daemons[daemon_index].binary, child_daemons[daemon_index].pid);
    }
}

void runChecker()
{
    while (1)
    {
        for (int i = 0; i < MAX_DAEMONS; i++)
        {
            int status;
            pid_t child_pid = waitpid(child_daemons[i].pid, &status, WNOHANG);
            if (child_pid > 0)
            {
                printf("Daemon %d (PID %d) has terminated. Restarting...\n", i, child_pid);
                runDaemon(child_daemons[i].uid, child_daemons[i].gid, child_daemons[i].binary, i);
            }
        }
        sleep(1);
    }
}

void forkChecker()
{
    pid_t child_pid;
    child_pid = fork();
    if (child_pid == -1)
    {
        perror("fork");
        exit(1);
    }
    else if (child_pid == 0)
    {
        runChecker();
    }
}

int main()
{
    signal(SIGTERM, sigterm_handler);
    signal(SIGINT, sigterm_handler);

    child_daemons[0].uid = 1001;
    child_daemons[0].gid = 1001;
    strcpy(child_daemons[0].binary, "/bin/httpd");

    child_daemons[1].uid = 1002;
    child_daemons[1].gid = 1002;
    strcpy(child_daemons[1].binary, "/bin/clid");

    for (int i = 0; i < MAX_DAEMONS; i++)
        runDaemon(child_daemons[i].uid, child_daemons[i].gid, child_daemons[i].binary, i);

    runChecker();

    // forkChecker();
    // execl("/bin/busybox", "su", "-", "ctf", NULL);

    return 0;
}