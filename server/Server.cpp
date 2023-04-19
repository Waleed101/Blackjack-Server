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

SocketServer *server;

int playerID = 0;
const int MAX_PLAYERS = 4;

struct Card
{
	std::string suit;
	int num;

	std::string toString() const
	{
		std::string str;
		switch (num)
		{
		case 1:
			str = "A";
			break;
		case 11:
			str = "J";
			break;
		case 12:
			str = "Q";
			break;
		case 13:
			str = "K";
			break;
		default:
			str = std::to_string(num);
			break;
		}
		return str + suit;
	}
};

int randomNum(int min, int max)
{
	return (rand() % (max - min + 1) + min);
}

Card getRandomCard()
{
	Card card;

	std::string suits[] = {"D", "H", "S", "C"};
	card.suit = suits[randomNum(0, 3)];
	card.num = randomNum(1, 13);

	return card;
}

// modify this method to change how cards are dealt
std::vector<Card> getCards(int numberOfCards, std::vector<Card> dealtCards)
{
	std::vector<Card> cards;

	for (int i = 0; i < numberOfCards; i++)
	{
		int count = 6;
		Card newCard;
		
		while (count == 6)
		{
			newCard = getRandomCard();
			count = 0;
			for (int i = 0; i < dealtCards.size(); i++)
			{
				if (dealtCards[i].num == newCard.num && dealtCards[i].suit == newCard.suit)
				{
					count++;
				}
			}
		}

		cards.push_back(newCard);
		dealtCards.push_back(newCard);
	}

	return cards;
}

Json::Value from(std::vector<Card> arr)
{
	Json::Value convertedArr(Json::arrayValue);

	for (Card inst : arr)
	{
		convertedArr.append(inst.toString());
	}

	return convertedArr;
}

std::vector<int> cardSum(std::vector<Card> cards) {
	std::vector<int> total = {0, 0};

	bool hasAce = false;

	for(Card card : cards) {
		if (card.num == 1) {
			hasAce = true;
			total[1] = total[0] + 10;
		}

		total[0] += std::min(card.num, 10);
		if (hasAce) {
			total[1] += std::min(card.num, 10);
		}
	}

	return total;
}

std::string formatCardSum(std::vector<int> cardSum) {
	return cardSum[1] != 0 ? (std::to_string(cardSum[0]) + "/" + std::to_string(cardSum[1])) : 
			std::to_string(cardSum[0]);
}

bool isBusted(std::vector<Card> cards) {
	std::vector<int> total = cardSum(cards);

	if (total[1] == 0) {
		return total[0] > 21;
	} else {
		return total[0] > 21 && total[1] > 21;
	}
}

bool doneTurn(std::vector<Card> cards, int max) {
	std::vector<int> total = cardSum(cards);

	return total[0] == max || total[1] == max || 
	(total[1] == 0 && total[0] > max) || 
	*min_element(total.begin(), total.end()) > max;
}

int getHigherTotal(std::vector<Card> cards) {
	std::vector<int> total = cardSum(cards);

	int actSum = total[0] > total[1] && total[0] <= 21 ? total[0] : total[1];

	return actSum;
}

struct Player {
    int id;
    int seat;
    int bet;
    std::vector<Card> cards;
    int balance;
    int isActive;
    int hasWon;
    
    Player(int p_id, int p_seat, int p_bet, std::vector<Card> p_cards, 
			int p_balance, int p_isActive, bool p_hasWon)
        : id(p_id), seat(p_seat), bet(p_bet), cards(p_cards),
          balance(p_balance), isActive(p_isActive), hasWon(p_hasWon)
    {}

	Json::Value toJson()
	{
		Json::Value converted(Json::objectValue);

		converted["seat"] = seat;
		converted["bet"] = bet;
		converted["cards"] = from(cards);
		converted["balance"] = balance;
		converted["isActive"] = isActive;
		converted["cardSum"] = formatCardSum(cardSum(cards));
		converted["isBusted"] = isBusted(cards);
		converted["hasWon"] = hasWon;

		return converted;
	}
};

struct Game {
	int gameID;
	std::vector<Player*> players;
	std::vector<Card> dealtCards;
	std::vector<Card> dealerCards;
	int currentState;
	int timeRemaining;
	int currentSeatPlaying;
	Json::Value gameState;
	Game(int id, int state, int timeRemaining, int currentSeat): gameID(id), currentState(state), 
	timeRemaining(timeRemaining), currentSeatPlaying(currentSeat)
	{}

	int getNumberOfPlayers() {
		return players.size();
	}

	void addPlayer(Player * newPlayer) {
		Semaphore mutex("mutex");

		mutex.Wait();

		this->players.push_back(newPlayer);
		
		for(int i = 0; i < this->getNumberOfPlayers(); i++) {
			this->players[i]->seat = i;
		}

		mutex.Signal();
		std::cout << "Seat: " << this->players[0]->seat << std::endl;
	}

	void removePlayer(int idToRemove) {
		Semaphore mutex("mutex");

		for (auto it = this->players.begin(); it != this->players.end(); ++it) {
			if ((*it)->id == idToRemove) {
				this->players.erase(it);
				break;
			}
		}
		
		for(int i = 0; i < this->getNumberOfPlayers(); i++) {
			this->players[i]->seat = i;
		}

		std::cout << "Removing player " << std::to_string(idToRemove) << " from Game#" << std::to_string(this->gameID) << std::endl;
	}
};

std::vector<Game*> games;


Json::Value from(std::vector<Player*> arr) {
	Json::Value convertedArr(Json::objectValue);

	for(Player * inst : arr) {
		convertedArr[std::to_string(inst->id)] = inst->toJson(); 
	}

	return convertedArr;
}

class DealerThread : public Thread{
	private:
		int TIME_BETWEEN_REFRESHES;
		int idx;
		//std::vector<Player*> players;

	public:
		DealerThread(int gameIdx):Thread(1000),TIME_BETWEEN_REFRESHES(1),idx(gameIdx) {
			// players = games[idx]->players;
			Thread::Start();
		}

		void updateGameState() {
			games[idx]->gameState["gameID"] = games[idx]->gameID;
			games[idx]->gameState["dealerCards"] = from(games[idx]->dealerCards);
			games[idx]->gameState["hasDealerBusted"] = isBusted(games[idx]->dealerCards);
			games[idx]->gameState["status"] = games[idx]->currentState;
			games[idx]->gameState["timeRemaining"] = games[idx]->timeRemaining;
			games[idx]->gameState["currentPlayerTurn"] = games[idx]->currentSeatPlaying;
			games[idx]->gameState["dealerSum"] = formatCardSum(cardSum(games[idx]->dealerCards));
			games[idx]->gameState["players"] = from(games[idx]->players);
			std::cout << games[idx]->gameState.toStyledString() << std::endl;
		}

		virtual long ThreadMain(void) override{
			Semaphore broadcast("broadcast");
			Semaphore mutex("mutex");
			while(true)
			{
								std::cout << "Player size: " << games[idx]->players.size() << std::endl;

				sleep(TIME_BETWEEN_REFRESHES);
				
				games[idx]->timeRemaining -= TIME_BETWEEN_REFRESHES;

				// mutex.Wait();

				std::cout << "A" << std::endl;
				if (games[idx]->timeRemaining <= 0) {	
					if (games[idx]->currentState == 1) {
						games[idx]->currentSeatPlaying++;
					}
				std::cout << "B" << std::endl;

					if (games[idx]->currentState == 0) {
						if (games[idx]->getNumberOfPlayers() > 0) {

							games[idx]->currentState = 1;
							if (games[idx]->dealtCards.size() >= 156)
							{
								games[idx]->dealtCards.clear();
							}

							games[idx]->dealerCards = getCards(2, games[idx]->dealtCards);
							std::cout << games[idx]->players.size() << std::endl;
							for (int i = 0; i < games[idx]->getNumberOfPlayers(); i++) {
								std::cout << "C" + i << std::endl;
								if (games[idx]->players[i]->isActive == 1)
									games[idx]->players[i]->isActive = 0;

								if (games[idx]->players[i]->isActive == 0)
									games[idx]->players[i]->cards = getCards(2, games[idx]->dealtCards);
								else if (games[idx]->players[i]->isActive == 2)
									games[idx]->removePlayer(games[idx]->players[i]->id);
							}
							std::cout << "D" << std::endl;
							games[idx]->currentSeatPlaying = 0;
						} else {
							if (games[idx]->currentSeatPlaying == games[idx]->getNumberOfPlayers()) {
								games[idx]->currentSeatPlaying = 1;
								games[idx]->currentState = 0;
								games[idx]->dealerCards = {};
							} else {
								games[idx]->currentSeatPlaying++;
							}
						}
									std::cout << "E" << std::endl;

						// timeRemaining = 10;
					}
					else if (games[idx]->currentState == 1 && games[idx]->currentSeatPlaying > games[idx]->getNumberOfPlayers()) {
						bool hasDealerBusted = isBusted(games[idx]->dealerCards);
						for (int i = 0; i < games[idx]->getNumberOfPlayers(); i++) {
							bool hasPlayerBusted = isBusted(games[idx]->players[i]->cards);
							if (!hasPlayerBusted && ((hasDealerBusted) || (getHigherTotal(games[idx]->players[i]->cards) > getHigherTotal(games[idx]->dealerCards)))) { // winner
								games[idx]->players[i]->hasWon = 1;
								games[idx]->players[i]->balance += games[idx]->players[i]->bet * 2;
							}
							else if(hasPlayerBusted || (!hasDealerBusted && (getHigherTotal(games[idx]->players[i]->cards) < getHigherTotal(games[idx]->dealerCards)))) { // loser
								games[idx]->players[i]->hasWon = 0;
								games[idx]->players[i]->balance -= games[idx]->players[i]->bet;
							} else { // push
								games[idx]->players[i]->hasWon = 2;
								games[idx]->players[i]->balance += games[idx]->players[i]->bet;
							}
						}

						// timeRemaining = 5;
						games[idx]->currentState = 2;
					} else if (games[idx]->currentState == 2) {
						games[idx]->currentSeatPlaying = 0;
						games[idx]->currentState = 0;
						games[idx]->dealerCards = {};
						// timeRemaining = 10;
					}
				std::cout << "F" << std::endl;

					games[idx]->timeRemaining = 10;
				}

				updateGameState();

				// mutex.Signal();

			
				for(int i = 0; i < games[idx]->getNumberOfPlayers(); i++) {
					broadcast.Signal();
				}
								std::cout << "G" << std::endl;
			}
		}
};

class PlayerReader : public Thread
{
	private:
		int playerID;
		int idx;
	public:
		Socket socket;
		
		PlayerReader(Socket & sock, int playerID, int gameIdx):Thread(1000),socket(sock),idx(gameIdx){
			this->playerID = playerID;
			Thread::Start();
		}
		
		virtual long ThreadMain(void) override{
			
			std::cout << "A player reader thread has started." << std::endl;
			
			Semaphore broadcast("broadcast");

			Json::Value initalBroadcast(Json::objectValue);

			// initalBroadcast["gameState"] = gameState;
			initalBroadcast["playerID"] = this->playerID;

			ByteArray responseBuffer(initalBroadcast.toStyledString());
			socket.Write(responseBuffer);
			
			while(true)
			{
				broadcast.Wait();
				std::cout << "Writing to socket..." << std::endl;
				std::cout << games[idx]->gameState.toStyledString() << std::endl;
				ByteArray responseBuffer(games[idx]->gameState.toStyledString());
				socket.Write(responseBuffer);
			}		
		}
};

class PlayerWriter : public Thread
{
	private:
		int playerID;
		int idx;

	public:
		Socket socket;
		Player data;

		PlayerWriter(Socket &sock, int playerID, int gameIdx)
			: Thread(1000), socket(sock),data(playerID, 0, 0, {}, 200, 1, false), idx(gameIdx)
		{
			Thread::Start();
		}
		
		virtual long ThreadMain(void) override{
			
			std::cout << "A player writer thread has started on Game #" << std::to_string(games[idx]->gameID) << std::endl;
			
			Semaphore mutex("mutex");
			
			Player * data_ptr = &data;
			games[idx]->addPlayer(data_ptr);			

			while (true)
			{
				ByteArray *buffer = new ByteArray();
				if (socket.Read(*buffer) == 0)
				{
					ByteArray * buffer = new ByteArray();
					if (socket.Read(*buffer) == 0) {
						std::cout << "Player-" << std::to_string(data.id) << " left Game #" << std::to_string(games[idx]->gameID) << std::endl;
						data.isActive = 2;
						break;
					}

				mutex.Wait();

				// // Modify the gameState has needed

				std::string req = (*buffer).ToString();
				Json::Value playerAction(Json::objectValue);
				Json::Reader reader;
				reader.parse(req, playerAction);

				if (playerAction["type"].asString() == "TURN")
				{
					std::string action = playerAction["action"].asString();
						std::cout << action << std::endl;
						if (action == "HIT") {
							data.cards.push_back(getRandomCard());
							
							if (doneTurn(data.cards, 21))
								games[idx]->currentSeatPlaying++;
						} else {
							games[idx]->currentSeatPlaying++;
						}
					} else {
						int betAmn = playerAction["betAmount"].asInt();
						data.balance -= betAmn; 
						data.bet = betAmn;
					}

				std::cout << playerAction.toStyledString() << std::endl;
				mutex.Signal();
			}
		}
	}
};

int main(int argc, char* argv[])
{
    std::cout << "-----C++ Server-----" << std::endl;

	bool hasSet = false;

    int port = argc >= 2 ? std::stoi(argv[1]) : 2000;

	while (!hasSet) {
		try {
			server = new SocketServer(port);
			hasSet = true;
		} catch (const std::string& e) {
    		std::cerr << "Caught exception: " << e << std::endl;
			port = rand() % (10000) + 1;
		}
	}

	int curGameID = 0;

	std::vector<DealerThread*> dealers = {};

	Game firstGame(curGameID++, 0, 10, 0);
	games.push_back(&firstGame);

	DealerThread * dealer = new DealerThread(0);
	dealers.push_back(dealer);

	Semaphore mutex("mutex", 1, true);
	Semaphore broadcast("broadcast", 0, true);

	bool hasJoined = false;

	std::cout << "Socket listening on " << port << std::endl;

	while (true)
	{
		try
		{
			Socket sock = server->Accept();
			int gameID;
			hasJoined = false;
			for(int i = 0; i < games.size(); i++) {
				if (games[i]->getNumberOfPlayers() < MAX_PLAYERS) {
					std::cout << std::to_string(i) << std::endl;
					std::cout << std::to_string(games[i]->getNumberOfPlayers()) << std::endl;
					gameID = i;
					hasJoined = true;
					break;
				}
			}

			if (!hasJoined) {
				std::cout << "All games were full. Creating a new game";
				Game* newGame = new Game(curGameID++, 0, 10, 0);
				games.push_back(newGame);

				gameID = curGameID - 1;
				while(games[gameID] == nullptr) {
					std::cout << ".";
				}

				std::cout << "" << std::endl;
				DealerThread * dealer = new DealerThread(gameID);
				dealers.push_back(dealer);
			}

			playerID++;
			std::cout << "GameID: " << gameID << std::endl;
			PlayerReader * reader = new PlayerReader(sock, playerID, gameID);
			PlayerWriter * writer = new PlayerWriter(sock, playerID, gameID);

    	} catch (std::string err) {
    		if (err == "Unexpected error in the server") {
    			std::cout << "Server is terminated" << std::endl;
    			break;
    		}
    	}
    }
    
    delete(server);
    
}
