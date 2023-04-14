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

Json::Value gameState(Json::objectValue);

int currentSeatPlaying = 0;

int numberOfPlayers = 0;
int playerID = 0;

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

std::vector<Card> dealtCards;

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
std::vector<Card> getCards(int numberOfCards)
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


std::vector<Player*> players;

Json::Value from(std::vector<Player*> arr) {
	Json::Value convertedArr(Json::objectValue);

	for(Player * inst : arr) {
		convertedArr[std::to_string(inst->id)] = inst->toJson(); 
	}

	return convertedArr;
}


void addPlayer(Player * newPlayer) {
	Semaphore mutex("mutex");
	mutex.Wait();
	players.push_back(newPlayer);
	mutex.Signal();
}

void removePlayer(int idToRemove) {
	Semaphore mutex("mutex");

	for (auto it = players.begin(); it != players.end(); ++it) {
		if ((*it)->id == idToRemove) {
			players.erase(it);
			break;
		}
	}

	numberOfPlayers--;

	std::cout << "Removing player " << std::to_string(idToRemove) << std::endl;
}

void incrementNextPlayer() {
	currentSeatPlaying++;
}
		
class DealerThread : public Thread{
	private:
		int TIME_BETWEEN_REFRESHES;
		
		std::vector<Card> cards; 

		int currentState = 0;
		int timeRemaining = 10;

	public:
		DealerThread():Thread(1000),TIME_BETWEEN_REFRESHES(1){

		}

		void updateGameState() {
			gameState["dealerCards"] = from(cards);
			gameState["hasDealerBusted"] = isBusted(cards);
			gameState["status"] = currentState;
			gameState["timeRemaining"] = timeRemaining;
			gameState["currentPlayerTurn"] = currentSeatPlaying;
			gameState["dealerSum"] = formatCardSum(cardSum(cards));
			gameState["players"] = from(players);
		}

		virtual long ThreadMain(void) override{
			Semaphore broadcast("broadcast", 0, true);
			Semaphore mutex("mutex");

			while(true)
			{
				sleep(TIME_BETWEEN_REFRESHES);
				timeRemaining -= TIME_BETWEEN_REFRESHES;

				mutex.Wait();

				if (timeRemaining <= 0) {	
					if (currentState == 1) {
						currentSeatPlaying++;
					}

					if (currentState == 0) {
						if (numberOfPlayers > 0) {

							currentState = 1;
							if (dealtCards.size() >= 156)
							{
								dealtCards.clear();
							}
							
							cards = getCards(2);

							for (int i = 0; i < players.size(); i++) {
								if (players[i]->isActive == 1)
									players[i]->isActive = 0;

								if (players[i]->isActive == 0)
									players[i]->cards = getCards(2);
								else if (players[i]->isActive == 2)
									removePlayer(players[i]->id);
							}

							currentSeatPlaying = 0;
						} else {
							if (currentSeatPlaying == numberOfPlayers) {
								currentSeatPlaying = 1;
								currentState = 0;
								cards = {};
							} else {
								currentSeatPlaying++;
							}
						}
					
						// timeRemaining = 10;
					}
					else if (currentState == 1 && currentSeatPlaying > numberOfPlayers) {
						bool hasDealerBusted = isBusted(cards);
						for (int i = 0; i < players.size(); i++) {
							bool hasPlayerBusted = isBusted(players[i]->cards);
							if (!hasPlayerBusted && ((hasDealerBusted) || (getHigherTotal(players[i]->cards) > getHigherTotal(cards)))) { // winner
								players[i]->hasWon = 1;
								players[i]->balance += players[i]->bet * 2;
							}
							else if(hasPlayerBusted || (!hasDealerBusted && (getHigherTotal(players[i]->cards) < getHigherTotal(cards)))) { // loser
								players[i]->hasWon = 0;
								players[i]->balance -= players[i]->bet;
							} else { // push
								players[i]->hasWon = 2;
								players[i]->balance += players[i]->bet;
							}
						}

						// timeRemaining = 5;
						currentState = 2;
					} else if (currentState == 2) {
						currentSeatPlaying = 0;
						currentState = 0;
						cards = {};
						// timeRemaining = 10;
					}

					timeRemaining = 10;
				}

				updateGameState();

				mutex.Signal();

				for(int i = 0; i < numberOfPlayers; i++) {
					broadcast.Signal();
				}
			}
		}
};

class PlayerReader : public Thread
{
	private:
		int playerID;
		DealerThread &dealer;
	public:
		Socket socket;
		
		PlayerReader(Socket & sock, int playerID, DealerThread &dealer):Thread(1000),socket(sock),dealer(dealer){
			this->playerID = playerID;
		}
		
		virtual long ThreadMain(void) override{
			
			std::cout << "A player reader thread has started." << std::endl;
			
			Semaphore broadcast("broadcast");

			Json::Value initalBroadcast(Json::objectValue);

			dealer.updateGameState();

			initalBroadcast["gameState"] = gameState;
			initalBroadcast["playerID"] = this->playerID;

			ByteArray responseBuffer(initalBroadcast.toStyledString());
			socket.Write(responseBuffer);
			
			while(true)
			{
				broadcast.Wait();
				std::cout << "Writing to socket..." << std::endl;
				std::cout << gameState.toStyledString() << std::endl;
				ByteArray responseBuffer(gameState.toStyledString());
				socket.Write(responseBuffer);
			}		
		}
};

class PlayerWriter : public Thread
{
	private:
		int playerID;
		DealerThread &dealer;

	public:
		Socket socket;
		Player data;

		PlayerWriter(Socket &sock, int playerID, DealerThread &dealer)
			: Thread(1000), socket(sock),
			data(playerID, 0, 0, {}, 200, 1, false), dealer(dealer)
		{}
		
		virtual long ThreadMain(void) override{
			
			std::cout << "A player writer thread has started." << std::endl;
			
			Semaphore mutex("mutex");
			
			Player * data_ptr = &data;
			addPlayer(data_ptr);			

			while (true)
			{
				ByteArray *buffer = new ByteArray();
				if (socket.Read(*buffer) == 0)
				{
					ByteArray * buffer = new ByteArray();
					if (socket.Read(*buffer) == 0) {
						std::cout << "Player-" << std::to_string(data.id) << " left the game." << std::endl;
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
								incrementNextPlayer();
						} else {
							incrementNextPlayer();
						}
					} else {
						int betAmn = playerAction["betAmount"].asInt();
						data.balance -= betAmn; 
						data.bet = betAmn;
					}

				std::cout << playerAction.toStyledString() << std::endl;
				std::cout << data.bet << std::endl;
				mutex.Signal();
			}
		}
	}
};

int main(int argc, char* argv[])
{
    std::cout << "-----C++ Server-----" << std::endl;

    int port = argc >= 2 ? std::stoi(argv[1]) : 2000;

	DealerThread * dealer = new DealerThread();
	
    
	Semaphore mutex("mutex", 1, true);

	server = new SocketServer(port);

	std::cout << "Socket listening on " << port << std::endl;

	while (true)
	{
		try
		{
			Socket sock = server->Accept();
			std::cout << "Got a new player" << std::endl;
			playerID++;
			numberOfPlayers++;
			PlayerReader * reader = new PlayerReader(sock, playerID, *dealer);
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
