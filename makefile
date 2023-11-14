FLAGS=-O3 -lm
CC=mpicc

nomePrograma=par


all: $(nomePrograma)

$(nomePrograma): paralel.o
	$(CC) -o $(nomePrograma) paralel.o $(FLAGS)

paralel.o: paralel.c
	$(CC) -c paralel.c $(FLAGS)

clean:
	rm -f *.o *.gch $(nomePrograma)