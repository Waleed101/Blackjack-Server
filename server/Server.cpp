#include "thread.h"
#include "socketserver.h"
#include "Semaphore.h"
#include <stdlib.h>
#include <time.h>
#include <list>
#include <vector>
#include <algorithm>
#include <jsoncpp/json/json.h>

using namespace Sync;

SocketServer * server;
Json::Value gameState;
Json::Value players;
int numberOfPlayers = 0;

class PlayerReader : public Thread{
	private:
		int playerID;
	public:
		Socket socket;
		
		PlayerReader(Socket & sock, int playerID):Thread(1000),socket(sock){
			playerID = playerID;
		}
		
		virtual long ThreadMain(void) override{
			
			std::cout << "A player reader thread has started." << std::endl;
			
			Semaphore broadcast("broadcast");

			ByteArray responseBuffer(gameState.toStyledString());
			socket.Write(responseBuffer);
			
			while(true)
			{
				broadcast.Wait();
				ByteArray responseBuffer(gameState.asString());
				socket.Write(responseBuffer);
			}		
		}
};

class PlayerWriter : public Thread{
	private:
		int playerID;
	public:
		Socket socket;
		
		PlayerWriter(Socket & sock, int playerID):Thread(1000),socket(sock){
			playerID = playerID;
		}
		
		virtual long ThreadMain(void) override{
			
			std::cout << "A player writer thread has started." << std::endl;
			
			Semaphore mutex("mutex");
			
			while(true)
			{
				mutex.Wait();
				
				// Modify the gameState has needed

				mutex.Signal();
			}		
		}
};


class DealerThread : public Thread{
	private:
		int TIME_BETWEEN_REFRESHES = 5000;

	public:
		DealerThread():Thread(1000){

		}
		
		virtual long ThreadMain(void) override{
			
			Semaphore broadcast("broadcast", 0, true);
			
			while(true)
			{
				for(int i = 0; i < numberOfPlayers; i++) {
					broadcast.Signal();
				}
				sleep(TIME_BETWEEN_REFRESHES);
			}
		}
};

void setGameState(int state, int timeRemaining) {
	std::string statusOptions[3] = {"BETTING", "PLAYING", "CLOSE"};
	gameState["status"] = statusOptions[state];
	gameState["timeRemaining"] = timeRemaining;
	gameState["players"] = players;
}

// // this can be used to add or change a player
// void updatePlayer(int playerID, )

int main(void)
{
    std::cout << "-----Server-----" << std::endl;
 
    int port = 2008;

	DealerThread * dealer = new DealerThread();
    
	Semaphore mutex("mutex", 1, true);

    server = new SocketServer(port);

	setGameState(1, 10);
    
	std::cout << "Socket listening on " << port << std::endl;
   
    while (true) {
    	try {	
 			Socket sock = server->Accept();
			std::cout << "Got a new player" << std::endl;
			PlayerReader * reader = new PlayerReader(sock, numberOfPlayers++);
			PlayerWriter * writer = new PlayerWriter(sock, numberOfPlayers);			
    	} catch (std::string err) {
    		if (err == "Unexpected error in the server") {
    			std::cout << "Server is terminated" << std::endl;
    			break;
    		}
    	}
    }
    
    delete(server);
    
}
