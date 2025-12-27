#include "csapp.h"
#include <time.h>
#include <string.h>
#include <stdlib.h>
#include <pthread.h>

#define MAX_CLIENTS 10
#define MAX_NAME_LEN 50
#define WORDS_COUNT 10

//Client Structure
 
// Holds active client information including:
// fd - client socket
// name - client's username
// wins - number of rounds won
// active - whether this client slot is in use

typedef struct 
{
    int fd;
    char name[MAX_NAME_LEN];
    int wins;
    int active;
} client_t;

client_t clients[MAX_CLIENTS];
int client_count = 0;
pthread_mutex_t clients_mutex = PTHREAD_MUTEX_INITIALIZER;

// Word list for rounds
char *words[WORDS_COUNT] = 
{
    "apple","banana","cherry","three","one","two","grape","kiwi","lemon","mango", "biology", "computer", "chemistry", "agriculture"
};

char current_word[50];
int round_active = 0;
char winner_name[MAX_NAME_LEN] = "";

// Sends a message to all currently active clients.
void broadcast(const char *msg) 
{
    pthread_mutex_lock(&clients_mutex);

    for (int i = 0; i < MAX_CLIENTS; i++) 
    {
        if (clients[i].active) 
        {
            write(clients[i].fd, msg, strlen(msg));
        }
    }

    pthread_mutex_unlock(&clients_mutex);
}

// Assembles and broadcasts the current leaderboard to all players.
void send_leaderboard() 
{
    char msg[MAXLINE];
    char board[MAXLINE] = "";

    pthread_mutex_lock(&clients_mutex);
    for (int i = 0; i < MAX_CLIENTS; i++) 
    {
        if (clients[i].active) 
        {
            char line[100];
            snprintf(line, sizeof(line), "%s: %d wins\n", clients[i].name, clients[i].wins);
            strcat(board, line);
        }
    }
    pthread_mutex_unlock(&clients_mutex);

    snprintf(msg, sizeof(msg), "\n--- Leaderboard ---\n%s", board);
    broadcast(msg);
}

// Selects a random word and tells all clients to begin typing.
void start_round() 
{
    pthread_mutex_lock(&clients_mutex);
    int idx = rand() % WORDS_COUNT;
    strcpy(current_word, words[idx]);
    round_active = 1;
    winner_name[0] = '\0';
    pthread_mutex_unlock(&clients_mutex);

    char msg[MAXLINE];
    snprintf(msg, sizeof(msg), "\n[New Word] Type this: %s\n", current_word);
    broadcast(msg);
}

// Ends the round, announces the winner (if any), and shows the leaderboard.
void end_round() 
{
    char msg[MAXLINE];

    pthread_mutex_lock(&clients_mutex);
    round_active = 0;

    if (winner_name[0] == '\0')
    {
        snprintf(msg, sizeof(msg), "\n[Round Result] No winner!\n");
    } else {

        snprintf(msg, sizeof(msg), "\n[Round Result] Winner is %s!\n", winner_name);
    }

    pthread_mutex_unlock(&clients_mutex);

    broadcast(msg);
    send_leaderboard();
}

// round_manager()
// Background thread that controls the game loop:
// 1. Waits 10 seconds
// 2. Starts round
// 3. Waits 10 seconds
// 4. Ends round
// Repeats forever.

void *round_manager(void *arg) 
{
    while (1) 
    {
        sleep(10);
        pthread_mutex_lock(&clients_mutex);
        int ok = client_count > 0;
        pthread_mutex_unlock(&clients_mutex);

        if (ok) {
            start_round();
            sleep(10);
            end_round();
        }
    }

    return NULL;
}

//client_thread()
//Handles all communication for one client:
// Ask for name
// Receive typed words
// Detect winner
// Handle disconnects

void *client_thread(void *arg) 
{
    int connfd = *((int *)arg);
    Free(arg);

    char message[MAXLINE];
    char name[MAX_NAME_LEN];

    // Ask player for name
    write(connfd, "Enter your name: ", 17);
    int n = read(connfd, name, MAX_NAME_LEN);

    if (n <= 0) 
    { 
        Close(connfd); return NULL; 
    }

    name[strcspn(name, "\n")] = 0;

    // Register client
    pthread_mutex_lock(&clients_mutex);
    int idx = -1;

    for (int i = 0; i < MAX_CLIENTS; i++) 
    {
        if (!clients[i].active) 
        {
            clients[i].fd = connfd;
            strncpy(clients[i].name, name, MAX_NAME_LEN);
            clients[i].wins = 0;
            clients[i].active = 1;
            idx = i;
            client_count++;
            break;
        }
    }

    pthread_mutex_unlock(&clients_mutex);

    if (idx == -1) 
    {
        write(connfd, "Server full!\n", 13);
        Close(connfd);
        return NULL;
    }

    // Welcome message
    snprintf(message, sizeof(message), "[Server] Welcome %s! Waiting for the next round...\n", name);
    write(connfd, message, strlen(message));

    // Main client loop: read user input
    while (1) 
    {
        n = read(connfd, message, MAXLINE);
        if (n <= 0) break;

        message[strcspn(message, "\n")] = 0;

        // Handle client quitting
        if (strcmp(message, "QUIT") == 0) 
        {
            write(connfd, "Server: Goodbye!\n", 18);
            break;
        }

        // Check for winning word
        pthread_mutex_lock(&clients_mutex);
        if (round_active && strcmp(message, current_word) == 0 && winner_name[0] == '\0') 
        {
            strncpy(winner_name, clients[idx].name, MAX_NAME_LEN);
            clients[idx].wins++;
        }

        pthread_mutex_unlock(&clients_mutex);
    }

    // Remove client on disconnect
    pthread_mutex_lock(&clients_mutex);
    clients[idx].active = 0;
    client_count--;
    pthread_mutex_unlock(&clients_mutex);

    Close(connfd);
    return NULL;
}

int main(int argc, char *argv[]) 
{
    if (argc != 2) 
    {
        fprintf(stderr, "Usage: %s <port>\n", argv[0]);
        exit(0);
    }

    srand(time(NULL));
    int listenfd = Open_listenfd(argv[1]);

    printf("Typing race server running...\n");

    // Start automatic round manager
    pthread_t round_tid;
    Pthread_create(&round_tid, NULL, round_manager, NULL);

    // Main accept loop
    // Waiting for new clients and launching a thread for each one.
    while (1) 
    {
        struct sockaddr_storage clientaddr;
        socklen_t clientlen = sizeof(clientaddr);

        int *connfdp = Malloc(sizeof(int));
        *connfdp = Accept(listenfd, (SA *)&clientaddr, &clientlen);

        pthread_t tid;
        Pthread_create(&tid, NULL, client_thread, connfdp);
    }

    return 0;
}
