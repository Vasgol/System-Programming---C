CC = gcc
CFLAGS = -g -O2
LIBS = -lm -lpthread
OBJECTS = main.o main_thread.o PList.o hb_pl_thread.o server_thread.o client_thread.o MD5_Lib.o BMP_Lib.o
          
peer: $(OBJECTS)
	$(CC) $(CFLAGS) $(OBJECTS) $(LIBS) -o peer 

main.o : main.c
	$(CC) $(CFLAGS) -c main.c

main_thread.o : main_thread.c
	$(CC) $(CFLAGS) -c main_thread.c

PList.o : PList.c
	$(CC) $(CFLAGS) -c PList.c

hb_pl_thread.o : hb_pl_thread.c
	$(CC) $(CFLAGS) -c hb_pl_thread.c

server_thread.o : server_thread.c
	$(CC) $(CFLAGS) -c server_thread.c

client_thread.o : client_thread.c
	$(CC) $(CFLAGS) -c client_thread.c

clean:                         # Cleaning Object Files and Executable, except from MD5_Lib.o, BMP_Lib.o
	rm -rf main.o main_thread.o PList.o hb_pl_thread.o server_thread.o client_thread.o peer


