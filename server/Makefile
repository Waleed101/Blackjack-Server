all: Client Server

Client : Client.o socket.o Blockable.o
	g++ -o Client Client.o socket.o Blockable.o -pthread -l rt -ljsoncpp

Client.o : Client.cpp socket.h
	g++ -c Client.cpp -std=c++11 -ljsoncpp

Server : Server.o thread.o socket.o socketserver.o Blockable.o
	g++ -o Server Server.o thread.o socket.o socketserver.o Blockable.o -pthread -l rt -ljsoncpp

Blockable.o : Blockable.h Blockable.cpp
	g++ -c Blockable.cpp -std=c++11 -ljsoncpp

Server.o : Server.cpp thread.h socketserver.h Semaphore.h
	g++ -c Server.cpp -std=c++11 -ljsoncpp -I.

thread.o : thread.cpp thread.h
	g++ -c thread.cpp -std=c++11 -ljsoncpp

socket.o : socket.cpp socket.h
	g++ -c socket.cpp -std=c++11 -ljsoncpp

socketserver.o : socketserver.cpp socket.h socketserver.h
	g++ -c socketserver.cpp -std=c++11 -ljsoncpp

clean:
	rm -f Client Server *.o