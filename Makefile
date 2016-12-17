CC=gcc
CFLAGS=-c 
MAIN=Threads_main.c Threads.h
FUNCTION=Threads.c Threads.h
MAINOBJ=Threads_main.o
FUNCTIONOBJ=Threads.o

Progetto: $(MAINOBJ) $(FUNCTIONOBJ)
	$(CC) Threads.o Threads_main.o -lpthread

Threads_main.o: $(MAIN)
	$(CC) $(CFLAGS) Threads_main.c -lpthread

Threads.o: $(FUNCTION)
	$(CC) $(CFLAGS) Threads.c -lpthread
