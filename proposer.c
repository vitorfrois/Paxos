#include "paxos.h"
#include <mpi.h>
#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>
#include <time.h>

#define NUM_SPAWNS 5
#define NUM_ACCEPTORS

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

void on_promise(Message received, MPI_Datatype message_struct, int sender,
                int proposal_id, int proposed_value, int **promises, int **data,
                int *num_promises) {
  if (received.promised_id != proposal_id)
    return;

  *promises[*num_promises] = received.accepted_id;
  *data[*num_promises] = received.data;
  *num_promises += 1;
  *promises = realloc(*promises, (*num_promises + 1) * sizeof(int));

  if (*num_promises == (int)((NUM_ACCEPTORS + 1) / 2)) { // majority
    int max_promise = -1, max_promise_index = -1;
    for (int i = 0; i < *num_promises; i++) {
      if (*promises[i] < max_promise) {
        max_promise = *promises[i];
        max_promise_index = i;
      }
    }

    int proposal_value =
        max_promise_index == -1 ? proposed_value : *data[max_promise_index];
    Message send;
    send.type = ACCEPT_REQUEST;
    send.data = proposal_value;
    send.promised_id = received.promised_id;
    send.accepted_id = -1;
    const int dst = sender;

    int n_processors;
    MPI_Comm_size(MPI_COMM_WORLD, &n_processors);
    for (int i = 0; i < n_processors; i++)
      MPI_Send(&send, 1, message_struct, i, 0, MPI_COMM_WORLD);
  }
}

bool on_accept(Message received, MPI_Datatype message_struct, int sender,
               int proposal_id, int proposed_value, int *num_promises) {

  if (received.promised_id != proposal_id)
    return false;

  *num_promises += 1;
  if (num_promises < (int)((NUM_ACCEPTORS + 1) / 2))
    return false;

  // majority
  Message send;
  send.type = CONSENSUS;
  send.data = proposed_value;
  send.accepted_id = proposal_id;
  int n_processors;
  MPI_Comm_size(MPI_COMM_WORLD, &n_processors);
  for (int i = 0; i < n_processors; i++)
    MPI_Send(&send, 1, message_struct, i, 0, MPI_COMM_WORLD);
  return true;
}

int main(int argc, char *argv[]) {
  MPI_Status status;
  MPI_Request request;
  MPI_Comm intercommunicator, parent_communicator;
  int errcodes[NUM_SPAWNS];
  int test_flag;
  MPI_Init(&argc, &argv);
  int root = 0;

  MPI_Datatype message_struct_type = create_message_struct();
  Message *receive_buffer = (Message *)malloc(sizeof(Message));

  MPI_Irecv(receive_buffer, sizeof(Message), message_struct_type,
            MPI_ANY_SOURCE, 0, MPI_COMM_WORLD, &request);

  bool consensus = false;
  int proposed_value; // proposed value
  int proposal_id;
  int acks = 0;                        // number of acks received from acceptors
  int *promises = malloc(sizeof(int)); // promises received from acceptors
  int *data = malloc(sizeof(int));     // data received from acceptors
  int num_promises = 0;

  proposed_value = 2;
  proposal_id = (int)time(NULL);

  // Send prepare for all acceptors
  Message send;
  send.type = PREPARE;
  send.data = -1;
  send.promised_id = proposal_id;
  send.accepted_id = -1;
  
  printf("Proposer: Sending PREPARE for all Acceptors");
  int n_processors;
  MPI_Comm_size(MPI_COMM_WORLD, &n_processors);
  for (int i = 0; i < n_processors; i++)
    MPI_Send(&send, 1, message_struct_type, i, TO_ACCEPTOR, MPI_COMM_WORLD);

  while (!consensus) {
    // the state of the proposer

    MPI_Test(&request, &test_flag, &status);
    if (test_flag) {
      switch (receive_buffer->type) {
      case PROMISE:
        printf("Proposer: Received PROMISE from Acceptor %d", status.MPI_SOURCE);
        on_promise(*receive_buffer, message_struct_type, status.MPI_SOURCE,
                   proposal_id, proposed_value, &promises, &data,
                   &num_promises);
        break;
      case ACCEPT:
        printf("Proposer: Received ACCEPT from Acceptor %d", status.MPI_SOURCE);
        on_accept(*receive_buffer, message_struct_type, status.MPI_SOURCE,
                  proposal_id, proposed_value, &num_promises);
        break;
      case CONSENSUS:
        printf("Proposer: Consensus Reached");
        consensus = true;
        break;
      }

      MPI_Irecv(receive_buffer, sizeof(Message), message_struct_type,
                MPI_ANY_SOURCE, FROM_ACCEPTOR, MPI_COMM_WORLD, &request);
    }
    // Wait for messages
  }

  MPI_Type_free(&message_struct_type);
  MPI_Finalize();
  free(promises);
  free(data);
  free(receive_buffer);

  return 0;
}
