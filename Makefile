CC=gcc
EXECUTABLE_NAME=sp
RM=rm -f

all: main.o 
	$(CC) main.c -o $(EXECUTABLE_NAME)

.PHONY: clean

clean:
	$(RM) $(EXECUTABLE_NAME).exe *.o
