export type Decision = {
  type: Type;
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
