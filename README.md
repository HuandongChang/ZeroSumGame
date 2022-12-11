# ZeroSumGame
## Introduction
In this project, we make a game called "the zero-sum mind reading game". Two players will be given the same n*n board with random generated numbers, and they are assigned to be the row player (loss money) and the column player (gain money) alternatively. The row player and the column player pick their choices independently trying to minimize their loss or maximize their gains. 

Authors: Huandong Chang, Eamon Worden, Ruizhe Fu, Kory Rosen

## Implementation
We build a distributed system with server and client for this game. In addition, since we have two users and allow users to communicate with each other, threads and thread synchronization are used.

Server: Server generates the board, calculate gain & loss, and pass messages between users.

Client: Interact with user, keep track of all game information, and receive user choices.


## Usage
Step 1: **make** to compile all files.

Step 2: run **./server [board dimension] [number of rounds]**. It will show the port number to which clients connect.

Step 3: two users connect to the server by running **./client [computer name] [port number]**.


## Demo
"Demo.mov" is a Video Demo, or you can watch it [here](https://www.youtube.com/watch?v=utmKtU2O34U).
