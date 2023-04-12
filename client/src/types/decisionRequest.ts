export type Decision = {
  type: Type;
  playerID: number;
  betAmount?: number | 0;
  action?: Action;
};

enum Type {
  "BET",
  "TURN",
}

enum Action {
  "HIT",
  "STAND",
}
