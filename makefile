FLAGS=-O3 -lm
CC=mpicc
NUM_PROCESSOS=2

nomePrograma=par

all: $(nomePrograma)

$(nomePrograma): paralel.o
	$(CC) -o $(nomePrograma) paralel.o $(FLAGS)

paralel.o: paralel.c
	$(CC) -c paralel.c $(FLAGS)

run: $(nomePrograma)
	mpirun -np $(NUM_PROCESSOS) ./$(nomePrograma) < input-edited

clean:
	rm -f *.o *.gch $(nomePrograma)