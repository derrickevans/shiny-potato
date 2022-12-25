CC=gcc
CFLAGS=-Wall
EXECUTABLE_NAME=sp
RM=rm -f

all: main.o 
	$(CC) $(CFLAGS) main.c -o $(EXECUTABLE_NAME)

.PHONY: clean

clean:
	$(RM) $(EXECUTABLE_NAME).exe *.o *.png
