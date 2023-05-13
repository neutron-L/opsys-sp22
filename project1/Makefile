
CC = gcc
CFLAGS = -O2 -g -Wall -Werror
PROGRAM = filecopy treecopy

all: $(PROGRAM)

filecopy: filecopy.o 
	$(CC) $(CFLAGS) -o $@ $^

treecopy: treecopy.o
	$(CC) $(CFLAGS) -o $@ $^

	
filecopy.o: filecopy.c 
	$(CC) $(CFLAGS) -c -o $@ $^

treecopy.o: treecopy.c
	$(CC) $(CFLAGS) -c -o $@ $^

clean:
	-rm *.o $(PROGRAM)