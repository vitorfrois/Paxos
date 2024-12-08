#ifndef PAXOS_H
#define PAXOS_H

// Message Type
#define PREPARE 0
#define PROMISE 1
#define ACCEPT_REQUEST 2
#define ACCEPT 3
#define CONSENSUS 4

// Message Tags
#define TO_ACCEPTOR 0
#define FROM_PROPOSER 0
#define TO_PROPOSER 1
#define FROM_ACCEPTOR 1

typedef struct StructMessage {
  int   promised_id;
  int   accepted_id;
  int   type;
  int   data;
} Message;

#endif 
