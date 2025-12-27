#include "csapp.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <pthread.h>

int clientfd;
pthread_mutex_t print_mutex = PTHREAD_MUTEX_INITIALIZER;


// recv_thread()

// Runs in the background and continuously reads messages sent by the server.
// This allows the client to type at any time without blocking server messages.

void *recv_thread(void *arg) 
{
    char message[MAXLINE];
    int n;
    while (1) 
    {
        n = read(clientfd, message, MAXLINE);
        if (n <= 0) break; // Lost connection → exit
        message[n] = '\0';

        pthread_mutex_lock(&print_mutex);
        printf("%s", message); // Print server message
        if (message[n-1] != '\n') printf("\n");
        pthread_mutex_unlock(&print_mutex);
    }
    exit(0); // Kill client if server disconnects
    return NULL;
}

int main(int argc, char *argv[]) 
{
    if (argc != 3) 
    {
        fprintf(stderr, "usage: %s <host> <port>\n", argv[0]);
        exit(0);
    }

    clientfd = Open_clientfd(argv[1], argv[2]);

    printf("Connected to typing race server!\n");
    printf("Type QUIT at any time to exit.\n\n");

    // Start background thread to listen for messages from server
    pthread_t tid;
    Pthread_create(&tid, NULL, recv_thread, NULL);

    char message[MAXLINE];

    
     // Main Input Loop
    
     // Reads user input and sends directly to server.
     // The server controls game flow; client simply sends typed lines.
    while (1) 
    {
        fgets(message, MAXLINE, stdin);

        if (message[0] != '\n') 
        {
            write(clientfd, message, strlen(message));
            if (strncmp(message, "QUIT", 4) == 0)
                break;
        }
    }

    Close(clientfd);
    return 0;
}
