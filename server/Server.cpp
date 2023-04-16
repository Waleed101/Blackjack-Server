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
};


Json::Value from(std::vector<Player*> arr) {
	Json::Value convertedArr(Json::objectValue);

	for(Player * inst : arr) {
		convertedArr[std::to_string(inst->id)] = inst->toJson(); 
	}

	return convertedArr;
}


void addPlayer(Player * newPlayer, Game* game) {
	Semaphore mutex("mutex");
	mutex.Wait();
	game->players.push_back(newPlayer);
	
	for(int i = 0; i < game->getNumberOfPlayers(); i++) {
		game->players[i]->seat = i;
	}

	mutex.Signal();
}

void removePlayer(int idToRemove, Game* game) {
	Semaphore mutex("mutex");

	for (auto it = game->players.begin(); it != game->players.end(); ++it) {
		if ((*it)->id == idToRemove) {
			game->players.erase(it);
			break;
		}
	}
	
	for(int i = 0; i < game->getNumberOfPlayers(); i++) {
		game->players[i]->seat = i;
	}

	std::cout << "Removing player " << std::to_string(idToRemove) << " from Game#" << std::to_string(game->gameID) << std::endl;
}

class DealerThread : public Thread{
	private:
		int TIME_BETWEEN_REFRESHES;
		Game* game;
		std::vector<Player*> players;

	public:
		DealerThread(Game* newGame):Thread(1000),TIME_BETWEEN_REFRESHES(1){
			game = newGame;
			players = game->players;
			Thread::Start();
		}

		void updateGameState() {
			game->gameState["gameID"] = game->gameID;
			game->gameState["dealerCards"] = from(game->dealerCards);
			game->gameState["hasDealerBusted"] = isBusted(game->dealerCards);
			game->gameState["status"] = game->currentState;
			game->gameState["timeRemaining"] = game->timeRemaining;
			game->gameState["currentPlayerTurn"] = game->currentSeatPlaying;
			game->gameState["dealerSum"] = formatCardSum(cardSum(game->dealerCards));
			game->gameState["players"] = from(game->players);
			// std::cout << game->gameState.toStyledString() << std::endl;
		}

		virtual long ThreadMain(void) override{
			Semaphore broadcast("broadcast");
			Semaphore mutex("mutex");
			

			while(true)
			{
				sleep(TIME_BETWEEN_REFRESHES);

				game->timeRemaining -= TIME_BETWEEN_REFRESHES;

				mutex.Wait();

				if (game->timeRemaining <= 0) {	
					if (game->currentState == 1) {
						game->currentSeatPlaying++;
					}

					if (game->currentState == 0) {
						if (game->getNumberOfPlayers() > 0) {

							game->currentState = 1;
							if (game->dealtCards.size() >= 156)
							{
								game->dealtCards.clear();
							}
							
							game->dealerCards = getCards(2, game->dealtCards);

							for (int i = 0; i < game->getNumberOfPlayers(); i++) {
								if (players[i]->isActive == 1)
									players[i]->isActive = 0;

								if (players[i]->isActive == 0)
									players[i]->cards = getCards(2, game->dealtCards);
								else if (players[i]->isActive == 2)
									removePlayer(players[i]->id, game);
							}

							game->currentSeatPlaying = 0;
						} else {
							if (game->currentSeatPlaying == game->getNumberOfPlayers()) {
								game->currentSeatPlaying = 1;
								game->currentState = 0;
								game->dealerCards = {};
							} else {
								game->currentSeatPlaying++;
							}
						}
					
						// timeRemaining = 10;
					}
					else if (game->currentState == 1 && game->currentSeatPlaying > game->getNumberOfPlayers()) {
						bool hasDealerBusted = isBusted(game->dealerCards);
						for (int i = 0; i < game->getNumberOfPlayers(); i++) {
							bool hasPlayerBusted = isBusted(game->players[i]->cards);
							if (!hasPlayerBusted && ((hasDealerBusted) || (getHigherTotal(players[i]->cards) > getHigherTotal(game->dealerCards)))) { // winner
								players[i]->hasWon = 1;
								players[i]->balance += game->players[i]->bet * 2;
							}
							else if(hasPlayerBusted || (!hasDealerBusted && (getHigherTotal(players[i]->cards) < getHigherTotal(game->dealerCards)))) { // loser
								players[i]->hasWon = 0;
								players[i]->balance -= players[i]->bet;
							} else { // push
								players[i]->hasWon = 2;
								players[i]->balance += players[i]->bet;
							}
						}

						// timeRemaining = 5;
						game->currentState = 2;
					} else if (game->currentState == 2) {
						game->currentSeatPlaying = 0;
						game->currentState = 0;
						game->dealerCards = {};
						// timeRemaining = 10;
					}

					game->timeRemaining = 10;
				}

				updateGameState();

				mutex.Signal();

			
				for(int i = 0; i < game->getNumberOfPlayers(); i++) {
					broadcast.Signal();
				}
			}
		}
};

class PlayerReader : public Thread
{
	private:
		int playerID;
		Game* game;
	public:
		Socket socket;
		
		PlayerReader(Socket & sock, int playerID, Game* newGame):Thread(1000),socket(sock){
			game = newGame;
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
				std::cout << game->gameState.toStyledString() << std::endl;
				ByteArray responseBuffer(game->gameState.toStyledString());
				socket.Write(responseBuffer);
			}		
		}
};

class PlayerWriter : public Thread
{
	private:
		int playerID;
		Game* game;

	public:
		Socket socket;
		Player data;

		PlayerWriter(Socket &sock, int playerID, Game* newGame)
			: Thread(1000), socket(sock),data(playerID, 0, 0, {}, 200, 1, false)
		{
			game = newGame;
			Thread::Start();
		}
		
		virtual long ThreadMain(void) override{
			
			std::cout << "A player writer thread has started on Game #" << std::to_string(game->gameID) << std::endl;
			
			Semaphore mutex("mutex");
			
			Player * data_ptr = &data;
			addPlayer(data_ptr, game);			

			while (true)
			{
				ByteArray *buffer = new ByteArray();
				if (socket.Read(*buffer) == 0)
				{
					ByteArray * buffer = new ByteArray();
					if (socket.Read(*buffer) == 0) {
						std::cout << "Player-" << std::to_string(data.id) << " left Game #" << std::to_string(game->gameID) << std::endl;
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
								game->currentSeatPlaying++;
						} else {
							game->currentSeatPlaying++;
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

    int port = argc >= 2 ? std::stoi(argv[1]) : 2000;

	int curGameID = 1;

	std::vector<Game*> games = {};
	std::vector<DealerThread*> dealers = {};

	Game firstGame(curGameID++, 0, 10, 0);
	games.push_back(&firstGame);

	DealerThread * dealer = new DealerThread(games[0]);
	dealers.push_back(dealer);

	Semaphore mutex("mutex", 1, true);
	Semaphore broadcast("broadcast", 0, true);

	server = new SocketServer(port);

	bool hasJoined = false;

	std::cout << "Socket listening on " << port << std::endl;

	while (true)
	{
		try
		{
			Socket sock = server->Accept();
			Game* game_ptr;
			hasJoined = false;

			for(int i = 0; i < games.size(); i++) {
				std::cout << "A" << std::endl;
				if (games[i]->getNumberOfPlayers() < MAX_PLAYERS) {
				std::cout << "B" << std::endl;
					game_ptr = games[i];
					hasJoined = true;
				std::cout << "C" << std::endl;
					break;
				}
			}

			if (!hasJoined) {
				std::cout << "All games were full. Creating a new game..." << std::endl;
				game_ptr = new Game(curGameID++, 0, 10, 0);
				games.push_back(game_ptr);
				DealerThread * dealer = new DealerThread(game_ptr);
				dealers.push_back(dealer);
			}

				std::cout << "D" << std::endl;
			playerID++;
			PlayerReader * reader = new PlayerReader(sock, playerID, game_ptr);
			PlayerWriter * writer = new PlayerWriter(sock, playerID, game_ptr);

    	} catch (std::string err) {
    		if (err == "Unexpected error in the server") {
    			std::cout << "Server is terminated" << std::endl;
    			break;
    		}
    	}
    }
    
    delete(server);
    
}
