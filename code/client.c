#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "message.h"
#include "socket.h"
#include <pthread.h>

int opponent_choice = -1;
int current_rowcol = -1;
int this_round_points = 0;

void* listen_server(void* args){
  int fd = *(int*)args;
  char* message;
  while(1){
    message = receive_message(fd);
    if (message == NULL) {
      perror("Exiting");
      exit(EXIT_FAILURE);
    }
    // printf("received %s", message);
    if (strncmp(message, "r:", 2) == 0){
      current_rowcol = atoi(message+2);
      // printf("rowcol: %d", current_rowcol);
      continue;
    }
    if (strncmp(message, "t:", 2) == 0){
      printf("\n%s\n", message);
      continue;
    }
    if (strncmp(message, "o:", 2) == 0){
      opponent_choice = atoi(message+2);
    }
    if (strncmp(message, "g:", 2) == 0){
      this_round_points = atoi(message+2);
    }
  }
}

/**
 * printBoards: 
 * This function prints the boards for the user
 * 
 * @param boards: the array of numbers representing the board
 * @param dim: the dimension of the boards
 */
void printBoards(int* boards, int dim) {
  printf("   |  ");
  for (int i = 0; i < dim; i++) {
    printf("%-4d", i);
  }
  printf("\n");

  for (int i = 0; i < dim * (dim + 1) + 6; i++) {
    printf("-");
  }
  printf("\n");

  for (int i = 0; i < dim; i++) {
    printf("%-3d", i);
    printf("|  ");
    for (int j = 0; j < dim; j++) {
      printf("%-4d", boards[i * dim + j]);
    }
    printf("\n");
  }
  printf("\n");
}

// main function
int main(int argc, char** argv) {
  if (argc != 3) {
    fprintf(stderr, "Usage: %s <server name> <port>\n", argv[0]);
    exit(EXIT_FAILURE);
  }

  // Read command line arguments
  char* server_name = argv[1];
  unsigned short port = atoi(argv[2]);

  // Connect to the server
  int socket_fd = socket_connect(server_name, port);
  if (socket_fd == -1) {
    perror("Failed to connect");
    exit(EXIT_FAILURE);
  }

  // receive the dimension and total rounds information
  int dim = atoi(receive_message(socket_fd));
  int rounds = atoi(receive_message(socket_fd));

  // receive the boards
  int boards[dim * dim];
  for (int i = 0; i < dim * dim; i++) {
    boards[i] = atoi(receive_message(socket_fd));
  }
  current_rowcol = atoi(receive_message(socket_fd));

  char* message;              // message to receive
  size_t size = 0;
  int current_points = 0;     // current total points
  int this_round_points = 0;  // this rounds points

  pthread_t thread;
  pthread_create(&thread, NULL, listen_server, & socket_fd);

  for (int i = 0; i < rounds; i++) {
    printf("-----------------This is round %d---------------------\n", i + 1);
    // receive information about whether gaining or losing
    // points in this round

    // case if user will receive points
    // while(current_rowcol == -1);
    if (current_rowcol == 0) {
      printf("You are column player, and will gain points in this round\n");
    }
    // else case if user will lose points
    else {
      printf("You are row player, and will lose points in this round\n");
    }
    printf("\n");

    // print boards for user
    printBoards(boards, dim);

    printf("Please enter which coloum/row you want to choose: ");
    int choice;
    // receive & verify their choice
    while (1){
      getline(&message, &size, stdin);
      // getline(&message, &size, stdin);

      // check whether user send a message or a choice
      if (strncmp(message, "t:", 2) == 0) {
        send_message(socket_fd, message);
        continue;
      }
      // if the choice is larger than dimension
      if (atoi(message) < dim && atoi(message) >=0){
        choice = atoi(message);
        send_message(socket_fd, message);
        while (opponent_choice == -1){}
        break;
      }
      else {
        printf("Invalid input. Please try again\n");
      }
    }

    // int rc = send_message(socket_fd, message);

    printf("\n");

    // receive the opponents information
    // if (opponent_choice == -1){
    //   continue;
    // }
    // else {
    //   // printf("opponent choice is %d\n", opponent_choice);
    // }
    // while (this_round_points == 0){}
    // opponent_choice = atoi(receive_message(socket_fd));
    // this_round_points = atoi(receive_message(socket_fd));
    // while (this_round_points == 0);
    // sleep(500);
    if (current_rowcol) {
      this_round_points = -boards[opponent_choice+dim*choice];
    }
    else {
      this_round_points = boards[opponent_choice*dim+choice];
    }
    current_points += this_round_points;

    // print all information in this round
    printf("Your opponent choose %d\n", opponent_choice);
    printf("You gain %d points in this round\n", this_round_points);
    printf("Your total points is %d now\n", current_points);

    printf("\n\n\n");
    opponent_choice = -1;
    current_rowcol = (current_rowcol+1)%2;
    this_round_points = 0;
  }

  // print win/lose information in the end
  if (current_points>0){
    printf("You win!\n");
  }
  else if (current_points == 0){
    printf("It's a tie!\n");
  }
  else{
    printf("You lose!\n");
  }

  // Free the message
  free(message);


  close(socket_fd);

  return 0;
}
