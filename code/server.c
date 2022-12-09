#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <ctype.h>
#include <pthread.h>
#include <time.h>
#include "message.h"
#include "socket.h"

int dim;
int rounds;

typedef struct thread_args {
  int client_socket_fd;
  int other_client;
  int round;
  int rowcol;
  int* choice;
} thread_args_t;

void * communicate(void *arg){
  thread_args_t *args = (thread_args_t*) arg;
  int fd = args->client_socket_fd;
  int fdo = args->other_client;

  // Send whether they are row player or col player in the this round;
  char row_col[4];
  sprintf(row_col,"%d",args->rowcol);
    int rc = send_message(args->client_socket_fd, row_col);
    if (rc == -1) {
      perror("Failed to send message to client");
      exit(EXIT_FAILURE);
    }

  //receive & verify their choice
   while(1){
    char* message = receive_message(args->client_socket_fd);
    if (message == NULL) {
      perror("Failed to read message from client");
      exit(EXIT_FAILURE);
    }
    //BEGIN CHANGES
    if (strncmp(message, "t:", 2) == 0) {
      //TODO IMPLEMENT CHAT
      printf("%s", message);
      send_message(fdo, message);
      // send_message(args->client_socket_fd, message);
      //SEND TO CLIENTS NOT SELF
      //NEED TO INCLUDE FD OF OTHERS
      continue;
    }
    int received_choice = atoi(message);
    //END CHANGES

    if (received_choice>=0 && received_choice<dim){
      *(args->choice) = received_choice;
      break;
    }
    //BEGIN CHANGES
    //TODO IMPLEMENT WARNING MESSAGE
    printf("Invalid move. Please try again\n");
    //END CHANGES
  
  }
  // close(args->client_socket_fd);
  free(args);
  
  return NULL;
}

int main(int argc, char** argv) {
  srand(time(NULL));
  // Unpack arguments
  if (argc == 3) {
    dim = atoi(argv[1]);
    //BEGIN CHANGES
    if (dim > 9){
      fprintf(stderr, "Dimension needs to be less than 9\n");
      exit(1);
    }
    if (dim < 2) {
      fprintf(stderr, "Dimension needs to be at least 2\n");
      exit(1);
    }
    rounds = atoi(argv[2]);
    if (rounds % 2){
      fprintf(stderr, "Rounds needs to be even\n");
      exit(2);
    }
    if (rounds > 10) {
      fprintf(stderr, "Max rounds is 10\n");
      exit(2);
    }
    if (rounds < 2){
      fprintf(stderr, "Min rounds is 2\n");
    }
  }
  //END CHANGES
  else{
    fprintf(stderr, "Usage: %s <dimension> <#rounds> \n", argv[0]);
    exit(1);
  }


  // Open a server socket
  unsigned short port = 0;
  int server_socket_fd = server_socket_open(&port);
  if (server_socket_fd == -1) {
    perror("Server socket was not opened");
    exit(EXIT_FAILURE);
  }

  // Start listening for connections, with a maximum of one queued connection
  if (listen(server_socket_fd, 1)) {
    perror("listen failed");
    exit(EXIT_FAILURE);
  }

  printf("Server listening on port %u\n", port);

  // Wait for two clients to connect
  int fd_arr[2];
  for (int i = 0; i < 2; i++) {
    fd_arr[i] = server_socket_accept(server_socket_fd);
    if (fd_arr[i] == -1) {
      perror("accept failed");
      exit(EXIT_FAILURE);
    }
  }

  // Send 1: send dimension and rounds to clients
  for(int i=0;i<2;i++){
    char dim_str[16];
    sprintf(dim_str,"%d",dim);
    int rc1 = send_message(fd_arr[i], dim_str);
    if(rc1 == -1) {
      perror("Failed to send message to client");
      exit(EXIT_FAILURE);
    }

    char rounds_str[16];
    sprintf(rounds_str,"%d",rounds);
    int rc2 = send_message(fd_arr[i], rounds_str);
    if(rc2 == -1) {
      perror("Failed to send message to client");
      exit(EXIT_FAILURE);
    }
  }


  // Generate board
  int board[dim*dim];
  for (int i=0;i<dim*dim;i++){
    board[i] = rand()%(3*dim*dim)+1;
  }
  printf("Done generating board\n");

  // Send 2: send board to clients
  for(int i=0;i<2;i++){
    for(int j=0;j<dim*dim;j++){
      char board_str[16];
      sprintf(board_str,"%d",board[j]);
      int rc = send_message(fd_arr[i], board_str);
      if (rc == -1) {
        perror("Failed to send message to client");
        exit(EXIT_FAILURE);
      }
    }
  }

  int round = 0;
  while(round<rounds){

  // create two threads
  pthread_t threads[2];
  // record user choices
  int user_choices[2];

  for(int i=0;i<2;i++){
    thread_args_t * args = malloc(sizeof(thread_args_t));
    args->client_socket_fd = fd_arr[i];
    args->other_client = fd_arr[(i+1)%2];
    args->round = round;
    // 0 is col, 1 is row.
    args->rowcol = (i+round) % 2;
    args->choice = &user_choices[i];
    if (pthread_create(&threads[i], NULL, communicate, args)){
      perror("pthread_create failed");
      exit(EXIT_FAILURE);
    }
  }

  // join two threads
  for(int i=0;i<2;i++){
    if (pthread_join(threads[i], NULL)) {
      perror("pthread_join failed");
      exit(EXIT_FAILURE);
    }
  }

      //Send 3: send opponent's choice
    for(int i=0;i<2;i++){
      char oppo_choice_str[16];
      sprintf(oppo_choice_str,"o:%d",user_choices[(i+1)%2]);
      printf("sending %s\n", oppo_choice_str);
  
          int rc = send_message(fd_arr[i], oppo_choice_str);
          if (rc == -1) {
            perror("Failed to send message to client");
            exit(EXIT_FAILURE);
          }
      }


    // Calculate gain & loss
    int value;
    // User 1 is the column player (gain money) in even rounds
    if (round%2 == 0){
      value = board[user_choices[1]*dim+user_choices[0]];
    }
    // User 2 is the column player (gain money) in odd rounds
    else{
      value = board[user_choices[0]*dim+user_choices[1]];
    }
    
    //Send 4: send gain & loss to clients
    for(int i=0;i<2;i++){
      char gain_loss_str[16];
      if ((i+round) % 2 == 0){
        sprintf(gain_loss_str,"g:%d",value);
      }
      else 
      {
        sprintf(gain_loss_str,"g:%d",value*(-1));
      }
        printf("gain loss %s\n", gain_loss_str);
        int rc = send_message(fd_arr[i], gain_loss_str);
        if (rc == -1) {
          perror("Failed to send message to client");
          exit(EXIT_FAILURE);
        }
      }

    round++;
  }
  sleep(500);
  close(server_socket_fd);

  return 0;
}
