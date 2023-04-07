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
				std::cout << "Writing to socket..." << std::endl;
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
		std::vector<std::string> cards; 
		int currentState = 0;
		bool hasDelt = false;

	public:
		DealerThread():Thread(1000){

		}
		
		virtual long ThreadMain(void) override{
			
			Semaphore broadcast("broadcast", 0, true);
			Semaphore mutex("mutex");
			
			while(true)
			{
				if (!hasDelt && currentState == 1) {
					hasDelt = true;
					cards.clear();
					cards.push_back("1");
					cards.push_back("2");
				}

				mutex.Wait();

				// make modifications to the game state

				mutex.Signal();

				for(int i = 0; i < numberOfPlayers; i++) {
					broadcast.Signal();
				}
				
				sleep(TIME_BETWEEN_REFRESHES);
			}
		}
};

void setGameState(int state, int timeRemaining, int turnID) {
	std::string statusOptions[3] = {"BETTING", "PLAYING", "CLOSE"};

	gameState["status"] = statusOptions[state];
	gameState["timeRemaining"] = timeRemaining;
	gameState["players"] = players;

	gameState["currentPlayerTurn"] = std::to_string(turnID);
}

Json::Value from(std::vector<std::string> arr) {
	Json::Value convertedArr(Json::arrayValue);

	for(std::string inst : arr) {
		convertedArr.append(inst);
	}

	return convertedArr;
}

bool isBusted(std::string * cards) {
	int total = 0;

	for(int i = 0; i < sizeof(cards); i++) {
		total += stoi(cards[i]);

		if (total > 21) {
			return false;
		}
	}

	return true;
}

int main(void)
{
    std::cout << "-----C++ Server-----" << std::endl;
 
    int port = 2008;

	DealerThread * dealer = new DealerThread();
    
	Semaphore mutex("mutex", 1, true);

    server = new SocketServer(port);

	setGameState(1, 10, 1);
    
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
