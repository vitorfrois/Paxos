#include "paxos.h"
#include <mpi.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define NUM_SPAWNS 5

MPI_Datatype create_message_struct() {
  const int n_items = 4;
  int blocklengths[4] = {1, 1, 1, 1};
  MPI_Datatype types[4] = {MPI_INT, MPI_INT, MPI_INT, MPI_INT};
  MPI_Datatype message_struct_type;
  MPI_Aint offsets[4];

  offsets[0] = offsetof(Message, promised_id);
  offsets[1] = offsetof(Message, accepted_id);
  offsets[2] = offsetof(Message, type);
  offsets[3] = offsetof(Message, data);

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

  MPI_Comm_spawn("acceptor", MPI_ARGV_NULL, 1, MPI_INFO_NULL,
                 root, MPI_COMM_SELF, &intercommunicator, errcodes);

  MPI_Comm_spawn("proposer", MPI_ARGV_NULL, 1, MPI_INFO_NULL,
                 root, MPI_COMM_SELF, &intercommunicator, errcodes);          

  printf("uai");

  MPI_Type_free(&message_struct_type);
  MPI_Finalize();

  printf("Timestamp: %d\n", (int)time(NULL));

  return 0;
}