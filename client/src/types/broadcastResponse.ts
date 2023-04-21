export type Broadcast = {
  status: BroadcastStatus;
  players: {
    [key: number]: Player;
  };
  gameID: number;
  dealerCards: Array<string>;
  dealerSum: string;
  hasDealerBusted: boolean;
  timeRemaining: number;
  currentPlayerTurn: number;
  msg?: {
    text: string;
    status: MessageStatus;
    timeToClose: number;
  };
};

enum BroadcastStatus {
  "BETTING",
  "PLAYING",
  "FINISH",
  "SERVER_CLOSED"
}

enum MessageStatus {
  "SUCCESS",
  "FAIL",
  "INFO",
}

export type Game = {
  status: BroadcastStatus;
  timeRemaining: number;
  hasDealerBusted: boolean;
  currentPlayerTurn: number;
  gameID: number;
  otherPlayers?: { [PLAYER_ID: number]: Player };
  msg?: {
    text: string;
    status: MessageStatus;
    timeToClose: number;
  };
};

export type Player = {
  seat: number; // a number 0 - 3
  name?: string;
  bet: number;
  cards: Array<string>; // 9S, 11H (number|suit)
  balance: number;
  isActive: number; // the marker for if the player is in the round
  isBusted: boolean;
  cardSum: string; //11/17
  hasWon: number
};

export type Dealer = {
  dealerCards: Array<string>;
  cardSum: string;
};
