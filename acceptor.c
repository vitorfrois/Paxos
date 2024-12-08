#include "paxos.h"
#include <mpi.h>
#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>

#define NUM_SPAWNS 1

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

void on_prepare(Message received, MPI_Datatype message_struct_type, int sender,
                int *promised_id, int *accepted_id, int *accepted_data) {
  if (*promised_id >= received.promised_id)
    return;

  *promised_id = received.promised_id;
  Message send;
  send.type = PROMISE;
  send.data = *accepted_data;
  send.accepted_id = *accepted_id;
  send.promised_id = *promised_id;
  const int dst = sender;
  MPI_Send(&send, 1, message_struct_type, dst, TO_PROPOSER, MPI_COMM_WORLD);
}

void on_accept_request(Message received, MPI_Datatype message_struct_type,
                       int sender, int *promised_id, int *accepted_id,
                       int *accepted_data) {
  if (*promised_id > received.promised_id)
    return;

  *promised_id = received.promised_id;
  *accepted_id = received.promised_id;
  *accepted_data = received.data;
  Message send;
  send.type = ACCEPT;
  send.data = *accepted_data;
  send.promised_id = *promised_id;
  const int dst = sender;

  // Reply the sender
  MPI_Send(&send, 1, message_struct_type, dst, TO_PROPOSER, MPI_COMM_WORLD);
}

int main(int argc, char *argv[]) {
  MPI_Status status;
  MPI_Request request;
  MPI_Comm intercommunicator, parent_communicator;
  MPI_Init(&argc, &argv);
  int errcodes[NUM_SPAWNS];
  int test_flag;
  int root = 0;

  MPI_Datatype message_struct_type = create_message_struct();
  Message *receive_buffer = (Message *)malloc(sizeof(Message));

  printf("Acceptor: Receiving\n");
  MPI_Irecv(receive_buffer, sizeof(Message), message_struct_type,
            MPI_ANY_SOURCE, FROM_PROPOSER, MPI_COMM_WORLD, &request);

  bool consensus = false;
  while (!consensus) {
    int accepted_id;   // the propsoal number of the value accepted by the
                       // acceptor
    int promised_id;   // the highest proposal number the acceptor has received
    int accepted_data; // the value accepted by the acceptor

    MPI_Test(&request, &test_flag, &status);
    if (test_flag) {

      switch (receive_buffer->type) {
      case PREPARE:
        printf("Acceptor: Received PREPARE from %d", status.MPI_SOURCE);
        on_prepare(*receive_buffer, message_struct_type, status.MPI_SOURCE,
                   &promised_id, &accepted_id, &accepted_data);
        break;
      case ACCEPT_REQUEST:
        printf("Acceptor: Received ACCEPT_REQUEST from %d", status.MPI_SOURCE);
        on_accept_request(*receive_buffer, message_struct_type,
                          status.MPI_SOURCE, &promised_id, &accepted_id,
                          &accepted_data);
        break;
      case CONSENSUS:
        printf("Acceptor: Consensus Reached");
        consensus = true;
        break;
      }

      MPI_Irecv(receive_buffer, sizeof(Message), message_struct_type,
                MPI_ANY_SOURCE, FROM_PROPOSER, MPI_COMM_WORLD, &request);
    }
  }

  MPI_Type_free(&message_struct_type);
  MPI_Finalize();
  free(receive_buffer);


  return 0;
}