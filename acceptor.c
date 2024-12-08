#include "paxos.h"
#include <mpi.h>
#include <stdbool.h>

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

void on_prepare(Message received, MPI_Datatype message_struct, int sender,
                int *promised_id, int *accepted_data) {
  if (*promised_id < received.id) {
    *promised_id = received.id;
    Message send;
    send.type = PROMISE;
    send.data = *accepted_data;
    send.id = *promised_id;
    const int dst = sender;
    MPI_Send(&send, 1, message_struct, dst, 0, MPI_COMM_WORLD);
  }
}

void on_accept_request(Message received, MPI_Datatype message_struct, int sender,
               int *promised_id, int *accepted_id, int *accepted_data) {
  if (*promised_id <= received.id) {
    *promised_id = received.id;
    *accepted_id = received.id;
    *accepted_data = received.data;
    Message send;
    send.type = ACCEPT;
    send.data = *accepted_data;
    send.id = *promised_id;
    const int dst = sender;
    // Reply the sender
    MPI_Send(&send, 1, message_struct, dst, 0, MPI_COMM_WORLD);
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
    int accepted_id;   // the propsoal number of the value accepted by the acceptor
    int promised_id;   // the highest proposal number the acceptor has received
    int accepted_data; // the value accepted by the acceptor
    
    // Wait for messages
  }

  MPI_Type_free(&message_struct_type);
  MPI_Finalize();

  return 0;
}