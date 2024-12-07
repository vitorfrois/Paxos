#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>
#include "paxos.h"

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