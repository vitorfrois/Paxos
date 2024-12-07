#include "paxos.h"
#include <mpi.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define NUM_SPAWNS 5

MPI_Datatype create_message_struct() {
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

int main(int argc, char *argv[]) {

  MPI_Status status;
  MPI_Comm intercommunicator, parent_communicator;
  int errcodes[NUM_SPAWNS];

  MPI_Init(&argc, &argv);

  int root = 0;

  MPI_Datatype message_struct_type = create_message_struct();


  
  MPI_Comm_spawn("acceptor", MPI_ARGV_NULL, NUM_SPAWNS, MPI_INFO_NULL,
                 root, MPI_COMM_WORLD, &intercommunicator, errcodes);

  MPI_Type_free(&message_struct_type);
  MPI_Finalize();

  printf("Timestamp: %d\n", (int)time(NULL));

  return 0;
}