CC = gcc
CFLAGS = -o
CLIBS  = -lcurses
OBJECTS = main.o Tables.o Ships.o Human.o Computer.o

battleship : $(OBJECTS)
	$(CC) $(CFLAGS) battleship $(OBJECTS) $(CLIBS)

main.o : main.c
	$(CC) -c main.c

Tables.o : Tables.c
	$(CC) -c Tables.c

Ships.o : Ships.c
	$(CC) -c Ships.c

Human.o : Human.c
	$(CC) -c Human.c

Computer.o : Computer.c
	$(CC) -c Computer.c

clean:                          # Cleaning Object Files and Executable
	rm -rf *.o battleship
