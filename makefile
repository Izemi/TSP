CC=gcc
CFLAGS=-Wall -pedantic -std=c17 -g3

TSP: location.o tsp.o
	${CC} ${CCFLAGS} -o $@ $^ -lm

tsp.o: tsp.c location.h tsp.h
