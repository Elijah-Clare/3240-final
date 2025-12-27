# 3240-final
## Overall Concept

For my project, I will make a text-based multiplayer typing race game. Multiple clients can
connect to a server and compete to type a randomly selected word as quickly as possible. After
each round, the winner is displayed along with a leaderboard, then a new word is sent for the
next race.

## Client Responsibilities

Each client connects to the server and receives the word to type from the server at the start of
each round. Accept user input and submit the typed word back to the server. Receive messages
from the server announcing the winner of the round. Display round results and prompt the user to
participate in the next round. Allow the user to disconnect cleanly with a QUIT command. The
client interface will run on the terminal.

## Server Responsibilities

The server will manage incoming client connections using threads. Each client runs in its own
thread. Select a random word for each typing race round. Send the word to all connected clients
and receive input from clients. Determine which client typed the word correctly first. Broadcast
the winner to all connected clients with a timer in between rounds. The server maintains a shared
game state, including the current target word, a flag indicating whether a winner has been
declared for the current round, and client name.

## Server Services

- startRound – Sends a random word to all clients for the current round.
- sendWord – Receives a client’s submission for the round.
- showWinner – Broadcasts the winner to all clients.
- leaderboard – Displays the number of rounds won per player.
- quit – Disconnects a client from the server.

## Language/Library

I plan on using C with the csapp.h and csapp.c library plus time.h for random word selection and
timing

## Demo Video
<https://youtu.be/MbbOwPxr4T8>
