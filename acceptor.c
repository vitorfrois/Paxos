#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>
#include "paxos.h"
#include <stdbool.h>

#define NUM_SPAWNS 5

MPI_Datatype create_message_struct(){
  const int n_items = 3;
  int blocklengths[3] = {1, 1, 1};
  MPI_Datatype types[3] = {MPI_INT, MPI_INT, MPI_INT};
  MPI_Datatype message_struct_type;
  MPI_Aint offsets[3];

  offsets[0] = offsetof(Message, id);
  offsets[1] = offsetof(Message, type);
  offsets[2] = offsetof(Message, data);

  MPI_Type_create_struct(n_items, blocklengths, offsets, types,
                         &message_struct_type);
  MPI_Type_commit(&message_struct_type);
  return message_struct_type;
}

void on_prepare(Message received, int *promised_num, ){
  if (*promised_num < received.id){
    *promised_num = received.id;
    
  }

}

int main(int argc, char *argv[]) {
  MPI_Status status;
  MPI_Comm intercommunicator, parent_communicator;
  int errcodes[NUM_SPAWNS];

  MPI_Init(&argc, &argv);

  int root = 0;

  MPI_Datatype message_struct_type = create_message_struct();
  
  bool consensus = false;
  while (!consensus) {
    int accepted_num;  // the propsoal number of the value accepted by the acceptor
    int promised_num; // the highest proposal number the acceptor has received
    int accepted_value; // the value accepted by the acceptor 

    /* 
    This function is called when the acceptor receives a prepare message from one of the proposers.
    */
    on prepare (n, sender) {
        if (promised_num < n) {
            promised_num = n; 
            Persist state on disk;
            Send <Promise, n, promise(accepted_num, accepted_value)> to sender; 
        } else {
            Send <Nack, n> to sender;
        }
    }

    /* 
    This function is called when the acceptor receives an accept message from one of the proposers.
    */
    on accept (n, v, sender) {
        if (promised_num <= n){
            promised_num = n; 
            accepted_num = n; 
            accepted_value = v; 
            Persist state on disk;
            Send <Accepted, n> to sender; 
        }else {
            Send <Nack, n> to sender;
        }
    }
  }

  MPI_Type_free(&message_struct_type);
  MPI_Finalize();

  return 0;
}