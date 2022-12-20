
CC=gcc
CFLAGS = -g -Wall
ALL_OBJFILES=Receiver Sender


all: $(ALL_OBJFILES)

Receiver: Receiver.c
	$(CC) $(CFLAGS) $(MAIN_OBJFILES) Receiver.c -o Receiver

Sender: Sender.c
	$(CC) $(CFLAGS) $(MAIN_OBJFILES) Sender.c -o Sender


.PHONY: clean

clean:
	rm -f *.o $(ALL_OBJFILES) 
