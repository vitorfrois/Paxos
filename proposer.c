#include "paxos.h"
#include <mpi.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

#define NUM_SPAWNS 5
#define NUM_ACCEPTORS

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

void on_promise(Message received, MPI_Datatype message_struct, int sender,
                int proposal_id, int **promises, int**data, int *num_promises) {
  if (received.id != proposal_id)
    return;

  *num_promises += 1;
  *promises = realloc(*promises, *num_promises * sizeof(int));
  *promises[*num_promises - 1] = received.id;
	*data[*num_promises - 1] = received.data;

  if (*num_promises == (int)(NUM_ACCEPTORS + 1) / 2) { // majority
    int max_promise = -1, max_promise_index = 0;
    for (int i = 0; i < *num_promises; i++){
      if (*promises[i] < max_promise){
        max_promise = *promises[i];
				max_promise_index = i; 
			}
		}

    int proposal_value = *data[max_promise_index];
		Message send;
    // send.type = ACCEPT;
    // send.data = *accepted_data;
    // send.id = *promised_id;
    // const int dst = sender;

		// MPI_Send(const void *buf, int count, message_struct, int dest, int tag, MPI_Comm comm)
  }
}

void on_accepted(Message received, MPI_Datatype message_struct, int sender,
                 int proposal_id, int **promises, int *num_promises, int *acks) {

  if (received.id != proposal_id)
    return;

  *acks += 1;
  if (acks == (int) (NUM_ACCEPTORS + 1) / 2) { // majority
		
		// MPI_Send(const void *buf, int count, message_struct, int dest, int tag, MPI_Comm comm)

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
    // the state of the proposer
    int proposed_value; // proposed value
    int proposal_id;
    int acks = 0; // number of acks received from acceptors
    int *promises = malloc(sizeof(int)); // promises received from acceptors
    int *data = malloc(sizeof(int)); // promises received from acceptors
    int num_promises = 1;

    proposed_value = 2; // valor proposto



    // Wait for messages
  }

  MPI_Type_free(&message_struct_type);
  MPI_Finalize();

  return 0;
}
