CC=g++

all: src
	$(CC) -o bin/rup.o -c src/rup.cpp
	ar cr bin/librup.a bin/rup.o
	rm bin/rup.o

clean:
	rm bin/librup.a
