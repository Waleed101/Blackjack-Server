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

Json::Value gameState(Json::objectValue);

int numberOfPlayers = 0;
int playerID = 0;

struct Card {
    std::string suit;
    int num;
    
    std::string toString() const {
        std::string str;
        switch (num) {
            case 1: str = "A"; break;
            case 11: str = "J"; break;
            case 12: str = "Q"; break;
            case 13: str = "K"; break;
            default: str = std::to_string(num); break;
        }
        return str + suit;
    }
};

int randomNum(int min, int max) {
	return (rand() % (max - min + 1) + min);
}

Card getRandomCard() {
	Card card;
	
	std::string suits[] = {"D", "H", "S", "C"};
	card.suit = suits[randomNum(0, 3)];
	card.num = randomNum(1, 13);

	return card;
}

// modify this method to change how cards are dealt
std::vector<Card> getCards(int numberOfCards) {
	std::vector<Card> cards;

	for (int i = 0; i < numberOfCards; i++) {
		cards.push_back(getRandomCard());
	}

	return cards;
}

Json::Value from(std::vector<Card> arr) {
	Json::Value convertedArr(Json::arrayValue);

	for(Card inst : arr) {
		convertedArr.append(inst.toString());
	}

	return convertedArr;
}


bool isBusted(std::vector<Card> cards) {
	int total = 0;

	for(Card card : cards) {
		total += std::min(card.num, 10);

		if (total > 21)
			return true;
	}

	return false;
}

struct Player {
    int id;
    int seat;
    int bet;
    std::vector<Card> cards;
    int balance;
    bool isActive;
    bool hasWon;
    
    Player(int p_id, int p_seat, int p_bet, std::vector<Card> p_cards, 
			int p_balance, bool p_isActive, bool p_hasWon)
        : id(p_id), seat(p_seat), bet(p_bet), cards(p_cards),
          balance(p_balance), isActive(p_isActive), hasWon(p_hasWon)
    {}

	Json::Value toJson() {
		Json::Value converted(Json::objectValue);

		converted["seat"] = seat;
		converted["bet"] = bet;
		converted["cards"] = from(cards);
		converted["balance"] = balance;
		converted["isActive"] = isActive;
		converted["isBusted"] = isBusted(cards);

		return converted;
	}
};


Json::Value from(std::vector<Player> arr) {
	Json::Value convertedArr(Json::objectValue);

	for(Player inst : arr) {
		convertedArr[std::to_string(inst.id)] = inst.toJson(); 
	}

	return convertedArr;
}

class DealerThread : public Thread{
	private:
		int TIME_BETWEEN_REFRESHES;
		
		std::vector<Card> cards; 
		std::vector<Player> players;

		int currentState = 0;

	public:
		DealerThread():Thread(1000),TIME_BETWEEN_REFRESHES(5){

		}

		void addPlayer(Player newPlayer) {
			Semaphore mutex("mutex");

			mutex.Wait();
			players.push_back(newPlayer);
			mutex.Signal();
		}

		void removePlayer(int idToRemove) {
			Semaphore mutex("mutex");

			mutex.Wait();
			for (auto it = players.begin(); it != players.end(); ++it) {
				if (it->id == idToRemove) {
					players.erase(it);
					break;
				}
			}
			mutex.Signal();

			numberOfPlayers--;

			std::cout << "Removing player " << std::to_string(idToRemove) << std::endl;
		}
		
		virtual long ThreadMain(void) override{
			
			Semaphore broadcast("broadcast", 0, true);
			Semaphore mutex("mutex");

			int timeRemaining = 10;
			int currentSeatPlaying = 0;
			
			while(true)
			{
				sleep(TIME_BETWEEN_REFRESHES);

				timeRemaining -= TIME_BETWEEN_REFRESHES;

				mutex.Wait();

				if (timeRemaining <= 0) {
					if (currentState == 0) {
						currentState = 1;

						cards = getCards(2);

						for (int i = 0; i < players.size(); i++) {
							players[i].cards = getCards(2);
						}

						currentSeatPlaying = 0;
					} else {
						if (currentSeatPlaying == (numberOfPlayers - 1)) {
							currentSeatPlaying = 0;
							currentState = 0;
							cards = {};
						} else {
							currentSeatPlaying++;
						}
					}
					timeRemaining = 10;
				}

				gameState["dealerCards"] = from(cards);
				gameState["hasDealerBusted"] = isBusted(cards);
				gameState["state"] = currentState;
				gameState["timeRemaining"] = timeRemaining;
				gameState["turnID"] = currentSeatPlaying;
				gameState["players"] = from(players);

				mutex.Signal();

				for(int i = 0; i < numberOfPlayers; i++) {
					broadcast.Signal();
				}
			}
		}
};


class PlayerReader : public Thread{
	private:
		int playerID;
	public:
		Socket socket;
		
		PlayerReader(Socket & sock, int playerID):Thread(1000),socket(sock){
			this->playerID = playerID;
		}
		
		virtual long ThreadMain(void) override{
			
			std::cout << "A player reader thread has started." << std::endl;
			
			Semaphore broadcast("broadcast");

			ByteArray responseBuffer(std::to_string(this->playerID));
			socket.Write(responseBuffer);
			
			while(true)
			{
				broadcast.Wait();
				std::cout << "Writing to socket..." << std::endl;
				ByteArray responseBuffer(gameState.toStyledString());
				socket.Write(responseBuffer);
			}		
		}
};

class PlayerWriter : public Thread{
	private:
		int playerID;
		DealerThread &dealer;

	public:
		Socket socket;
		Player data;

		PlayerWriter(Socket &sock, int playerID, DealerThread &dealer)
			: Thread(1000), socket(sock),
			data(playerID, 0, 0, {}, 200, true, false), dealer(dealer)
		{}
		
		virtual long ThreadMain(void) override{
			
			std::cout << "A player writer thread has started." << std::endl;
			
			Semaphore mutex("mutex");
			
			dealer.addPlayer(data);			

			while(true)
			{
				ByteArray * buffer = new ByteArray();
				if (socket.Read(*buffer) == 0) {
					std::cout << "Player-" << std::to_string(data.id) << " left the game." << std::endl;
					dealer.removePlayer(data.id);
					break;
				}

				mutex.Wait();
				
				// // Modify the gameState has needed

				std::string data = (* buffer).ToString();

				Json::Value playerAction(Json::objectValue);

				Json::Reader reader;

				reader.parse(data, playerAction);

				std::cout << playerAction.toStyledString() << std::endl;
				mutex.Signal();
			}		
		}
};

int main(void)
{
    std::cout << "-----C++ Server-----" << std::endl;
 
<<<<<<< HEAD
    int port = 2040;
=======
    int port = 2041;
>>>>>>> 2ffaf5b1e9e5b75b215718ebdaf39695ac1b93fd

	DealerThread * dealer = new DealerThread();
    
	Semaphore mutex("mutex", 1, true);

    server = new SocketServer(port);
    
	std::cout << "Socket listening on " << port << std::endl;
   
    while (true) {
    	try {	
 			Socket sock = server->Accept();
			std::cout << "Got a new player" << std::endl;
			playerID++;
			numberOfPlayers++;
			PlayerReader * reader = new PlayerReader(sock, playerID);
			PlayerWriter * writer = new PlayerWriter(sock, playerID, *dealer);
    	} catch (std::string err) {
    		if (err == "Unexpected error in the server") {
    			std::cout << "Server is terminated" << std::endl;
    			break;
    		}
    	}
    }
    
    delete(server);
    
}
