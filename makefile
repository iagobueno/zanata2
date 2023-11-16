FLAGS=-O3 -lm
CC=mpicc
NUM_PROCESSOS=6

nomePrograma=par

all: $(nomePrograma)

$(nomePrograma): paralel.o
	$(CC) -o $(nomePrograma) paralel.o $(FLAGS)

paralel.o: paralel.c
	$(CC) -c paralel.c $(FLAGS)

run: $(nomePrograma)
	mpirun -np $(NUM_PROCESSOS) ./$(nomePrograma) < input

clean:
	rm -f *.o *.gch $(nomePrograma)