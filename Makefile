CC=mpicc

all:
	$(CC) main.c -o main
	$(CC) proposer.c -o proposer
	$(CC) acceptor.c -o acceptor
run:
	mpiexec -np 1 main --oversubscribe 