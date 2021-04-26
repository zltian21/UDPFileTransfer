CC=gcc
CFLAGS=-Wall

FILENAME = sample.txt
IP = 127.0.0.1
PORT = 9999

all: Server Client
	rm -f out.txt

p0t5: Server
	./Server $(PORT) 5 0

p2t5: Server
	./Server $(PORT) 5 0.2

p2t4: Server
	./Server $(PORT) 4 0.2

a0: Client
	./Client $(IP) $(PORT) $(FILENAME) 0

a2: Client
	./Client $(IP) $(PORT) $(FILENAME) 0.2

Server: Server.o DieWithError.o HandleClient.o Simulation.o

Client: Client.o DieWithError.o Simulation.o

DieWithError.o: DieWithError.c
	CC -c DieWithError.c

Simulation.o: Simulation.c
	CC -c Simulation.c

Server.o: Server.c packet.h
	CC -c Server.c

HandleClient.o: HandleClient.c packet.h
	CC -c HandleClient.c

Client.o: Client.c packet.h
	CC -c Client.c

clean:
	rm -f Server Client out.txt *.o