export type Broadcast = {
  status: BroadcastStatus;
  players: {
    [key: number]: Player;
  };
  dealerCards: [string];
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
  cards: [string]; // 9S, 11H (number|suit)
  balance: number;
  isActive: boolean; // the marker for if the player is in the round
  hasWon: boolean;
  cardSum: string;
};

export type Dealer = {
  dealerCards: [string];
  cardSum: string;
};
