export const bettingState = {
  status: 0,
  players: {
    1: {
      seat: 3, // a number 0 - 3
      bet: 0,
      cards: [""], // 9S, 11H (number|suit)
      balance: 200,
      isActive: true, // the marker for if the player is in the round
      hasWon: false,
      cardSum: "0",
    },
  },
  dealerCards: [""],
  dealerSum: "0",
  hasDealerBusted: false,
  timeRemaining: 10,
  currentPlayerTurn: 1,
};

export const playingState = {
  status: 1,
  players: {
    1: {
      seat: 2, // a number 0 - 3
      bet: 25,
      cards: ["10H", "8H"], // 9S, 11H (number|suit)
      balance: 200,
      isActive: true, // the marker for if the player is in the round
      hasWon: false,
      cardSum: "18",
    },
    2: {
      seat: 3, // a number 0 - 3
      bet: 25,
      cards: ["10H", "8H"], // 9S, 11H (number|suit)
      balance: 200,
      isActive: true, // the marker for if the player is in the round
      hasWon: false,
      cardSum: "18",
    },
  },
  dealerCards: ["10H", "8H"],
  dealerSum: "18",
  hasDealerBusted: false,
  timeRemaining: 10,
  currentPlayerTurn: 1,
};

export const tempGameState = {
  status: 1,
  timeRemaining: 0,
  hasDealerBusted: false,
  currentPlayerTurn: 0,
  otherPlayers: {
    1: {
      seat: 1, // a number 0 - 3
      bet: 25,
      cards: ["9H", "9H"], // 9S, 11H (number|suit)
      balance: 200,
      isActive: true, // the marker for if the player is in the round
      hasWon: false,
      cardSum: "18",
    },
    2: {
      seat: 2, // a number 0 - 3
      bet: 25,
      cards: ["8H", "9H", "9H"], // 9S, 11H (number|suit)
      balance: 200,
      isActive: true, // the marker for if the player is in the round
      hasWon: false,
      cardSum: "18",
    },
    3: {
      seat: 3, // a number 0 - 3
      bet: 25,
      cards: ["7H", "8H", "9H", "9H"], // 9S, 11H (number|suit)
      balance: 200,
      isActive: true, // the marker for if the player is in the round
      hasWon: false,
      cardSum: "18",
    },
    4: {
      seat: 4, // a number 0 - 3
      bet: 25,
      cards: ["6H"], // 9S, 11H (number|suit)
      balance: 200,
      isActive: true, // the marker for if the player is in the round
      hasWon: false,
      cardSum: "18",
    },
  },
};
