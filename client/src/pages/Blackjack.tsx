import { useEffect, useState } from "react";
import { useInterval } from "../hooks/useInterval";
import { URL, POLL_REFRESH_INTERVAL } from "../constants/constants";
import axios from "axios";

//types
import { Card } from "../types/card";
import { Broadcast, Player, Game, Dealer } from "../types/broadcastResponse";
import { Decision } from "../types/decisionRequest";
import {
  bettingState,
  playingState,
  tempGameState,
} from "../mockRequests/response";

// components
import Controls from "../components/Controls";
import Betting from "../components/Betting";
import PlayersHand from "../components/PlayersHand";
import OpponentsHand from "../components/OpponentsHand";
import { pid } from "process";

function Blackjack() {
  // is player connected to the server
  const [isConnected, setIsConnected] = useState(false);

  // the game related state
  const [gameState, setGameState] = useState<Game>({
    status: 0,
    timeRemaining: 0,
    hasDealerBusted: false,
    currentPlayerTurn: 4,
  });

  // player's state
  const [playerID, setPlayerID] = useState(0);
  const [player, setPlayer] = useState<Player>({
    bet: 0,
    seat: -1,
    cards: ["8H", "10C"],
    balance: 200,
    isActive: false,
    hasWon: false,
    cardSum: "0",
  });

  // dealer
  const [dealer, setDealer] = useState<Dealer>({
    dealerCards: ["", ""],
    cardSum: "0",
  });

  // seats
  const [seats, setSeats] = useState([1, 2, 3, -1]);

  // timer for move
  const [action, setAction] = useState("STAND");
  const [canPlay, setCanPlay] = useState(true);

  // loading switch
  const [stopLoading, setStopLoading] = useState(false);

  // timeoutFunction
  let timer: NodeJS.Timer;

  // define a winner
  const [winner, setWinner] = useState<"dealer" | "you" | "tie" | "">("");

  // bet handler
  const handleControlDecision = (choice: string) => {
    // clear button -> amount == 0
    if (choice === "CLEAR") {
      setPlayer({ ...player, bet: 0 });
    } else {
      // handle player actions
      setCanPlay(false);
      setAction(action);
    }
  };

  // server polling for game updates
  /*
  useInterval(async () => {
    const data = await axios.get(`/update/${playerID}`);
    const gameUpdate: Broadcast = data.data;

    // search through players an assign seats
    const otherPlayers: { [key: number]: Player } = {};
    const tempSeats = [-1, -1, -1, -1];
    for (let pKey in gameUpdate["players"]) {
      tempSeats[gameUpdate["players"][pKey]["seat"]] = parseInt(pKey);
      if (parseInt(pKey) !== playerID) {
        otherPlayers[pKey] = gameUpdate["players"][pKey];
      }
    }

    // update dealer states
    setDealer({
      dealerCards: gameUpdate["dealerCards"],
      cardSum: gameUpdate["dealerSum"],
    });

    // update player states
    const playerRef = gameUpdate["players"][playerID];
    setPlayer(playerRef);

    // update game states
    let tempGameState = {
      status: gameUpdate["status"],
      timeRemaining: gameUpdate["timeRemaining"],
      hasDealerBusted: gameUpdate["hasDealerBusted"],
      currentPlayerTurn: gameUpdate["currentPlayerTurn"],
    };

    if (Object.keys(otherPlayers).length > 0) {
      tempGameState = { ...tempGameState, ...{ otherPlayers } };
    }
    setGameState(tempGameState);

    // update the seats
    setSeats(tempSeats);
  }, POLL_REFRESH_INTERVAL);
  */

  // connect to game, fetch table id + table state
  const initialConnection = async () => {
    // const connectionData = await axios.get(URL + "/connect");
    // const givenId = connectionData.data.id;
    // setPlayerID(givenId);
    // const gameData = await axios.get(URL + `/update/${givenId}`);
    // setGameState(gameData.data);

    //TESTING
    setPlayerID(1);
    setGameState(tempGameState);
  };

  useEffect(() => {
    if (gameState["status"] === 0) {
      setStopLoading(true);
      timer = setTimeout(async () => {
        setStopLoading(false);

        // make the call at the end of the turn
        await axios.post(URL + `actions/${playerID}`, {
          type: "BET",
          betAmount: player["bet"],
        });
      }, 10000);
    }
  }, [gameState["status"]]);

  // controls the timer during playing state
  useEffect(() => {
    if (gameState["currentPlayerTurn"] === playerID) {
      // start the timer and wait till it finishes to set next actions
      setStopLoading(true);
      timer = setTimeout(async () => {
        setStopLoading(false);
        setCanPlay(true); // allow player to hit/stand again after timer

        // make the call at the end of the turn
        await axios.post(URL + `actions/${playerID}`, {
          type: "TURN",
          action: action,
        });
      }, 10000);
    }
  }, [player["cardSum"], gameState["currentPlayerTurn"]]);

  useEffect(() => {
    setIsConnected(false);
    initialConnection();
    setIsConnected(true);
  }, []);

  return (
    <div className="w-96 max-w-sm h-screen flex items-center flex-col">
      {!isConnected ? (
        <div>connecting to the table</div>
      ) : (
        <>
          {/* is game in betting state */}
          {gameState["status"] === 0 ? (
            <>
              <h1 className="text-6xl font-bold mb-8 mt-8">BETTING TIME...</h1>
              <h1 className="text-2xl font-bold mb-32 ">
                select a chip to place a bet
              </h1>
            </>
          ) : (
            <>
              <h1 className="text-3xl text-primary">DEALER</h1>
              {/* dealer hand */}
              <div
                id="dealerHand"
                className="flex justify-center mt-14 relative scale-[0.8] flex-wrap w-[300px]"
              >
                <h1
                  className={`text-3xl text-primary absolute -top-8 -right-10 ${
                    winner == "you" ? "line-through" : ""
                  } `}
                >
                  {/* more logic can go here to strike through value when busted */}
                  {dealer["cardSum"]}
                </h1>

                {dealer["dealerCards"].map((card, index) => {
                  // kinda janky logic
                  let cardArr = card.split("");
                  const suit = cardArr.pop();
                  const cardNo = cardArr.join("");

                  if (gameState["status"] === 1) {
                    const hidden =
                      index == 0
                        ? "bg-[url('/src/assets/cards/cardBack.png')] -rotate-6 -mt-3"
                        : "bg-[url('/src/assets/cards/card.png')]";
                    return (
                      <div
                        key={index}
                        className={`flex justify-center items-center text-4xl text-[#6D5C5C]  bg-cover w-24 h-36 -ml-6 ${hidden}`}
                      >
                        {index != 0 ? cardNo : ""}
                      </div>
                    );
                  } else {
                    const sendToAbove =
                      index > 2
                        ? `absolute left-${index > 3 ? 12 : 6} top-28`
                        : "";
                    const animateNewCard = index > 1 ? "animate-getCard" : "";
                    return (
                      <div
                        key={index}
                        className={`flex justify-center items-center text-4xl text-[#6D5C5C] bg-[url('/src/assets/cards/card.png')] bg-cover w-24 h-36 -ml-6 ${animateNewCard} ${sendToAbove}`}
                      >
                        {cardNo}
                      </div>
                    );
                  }
                })}
              </div>
              <div className="bg-[url('/src/assets/cards/dealerShadow.png')] bg-cover bg-inherit bg-no-repeat w-40 h-4 mt-10 opacity-25 blur-sm"></div>
            </>
          )}

          {/* Table Display */}
          <div id="table" className="flex absolute bottom-64">
            {seats.map((pId, index) => {
              console.log(index, pId);
              if (gameState.currentPlayerTurn != 0) {
                let pIdTurn = gameState.currentPlayerTurn;
                return <div className={`spot${pIdTurn} animate-ping`}></div>;
              }
              if (pId === playerID) {
                return (
                  // work here to add some nice background, maybe darken with text & cross?

                  <>
                    <div className="spot1 animate-ping w-20 h-32 border-solid border-slate-100 rounded-md border-4 opacity-50">
                      My hand
                    </div>
                  </>
                );
              } else if (pId !== -1) {
                console.log(index);
                // pass the opponents pId and associated cards here!

                return (
                  <>
                    {pId === gameState["currentPlayerTurn"] ? (
                      // MAKE THIS INTO A DOT, mark active player
                      <></>
                    ) : null}

                    {/* this is where the opponents hand is rendered it needs the styling  */}

                    {/* <div className="relative scale-[0.7] transform: rotate(4deg) translate(-200px) rotate(8deg) ;"> */}
                    {gameState.otherPlayers ? (
                      // Object.values(gameState.otherPlayers).map((player) => {

                      // })

                      gameState.otherPlayers[pId].seat === 1 &&
                      gameState.otherPlayers[pId].isActive != false ? (
                        <div
                          className={`spot1 flex justify-center items center`}
                        >
                          <OpponentsHand
                            cards={
                              gameState.otherPlayers
                                ? gameState.otherPlayers[pId]["cards"]
                                : [""]
                            }
                          />
                        </div>
                      ) : gameState.otherPlayers[pId].seat === 2 &&
                        gameState.otherPlayers[pId].isActive != false ? (
                        <div
                          className={`spot2 flex justify-center items center`}
                        >
                          <OpponentsHand
                            cards={
                              gameState.otherPlayers
                                ? gameState.otherPlayers[pId]["cards"]
                                : [""]
                            }
                          />
                        </div>
                      ) : gameState.otherPlayers[pId].seat === 3 &&
                        gameState.otherPlayers[pId].isActive != false ? (
                        <div
                          className={`spot3 flex justify-center items center`}
                        >
                          <OpponentsHand
                            cards={
                              gameState.otherPlayers
                                ? gameState.otherPlayers[pId]["cards"]
                                : [""]
                            }
                          />
                        </div>
                      ) : gameState.otherPlayers[pId].seat === 4 &&
                        gameState.otherPlayers[pId].isActive != false ? (
                        <div
                          className={`spot4 flex justify-center items center`}
                        >
                          <OpponentsHand
                            cards={
                              gameState.otherPlayers
                                ? gameState.otherPlayers[pId]["cards"]
                                : [""]
                            }
                          />
                        </div>
                      ) : null
                    ) : null}
                  </>
                );
              } else if (pId === -1) {
                let spotNum;
                spotNum = index;
                return (
                  <>
                    <div
                      className={`spot${index} w-20 h-32 border-solid border-slate-100 rounded-md border-4 opacity-50`}
                    >
                      Nobody to play with :/
                    </div>
                  </>
                );
              }
            })}
          </div>

          {/* is gameState in the betting state */}
          {gameState?.status === 0 ? (
            <>
              <div className="bg-[url('/src/assets/poker_chip_bg.png')] bg-inherit bg-no-repeat bg-cover w-48 h-48 opacity-50 flex justify-center items-center	">
                {/* COULD PUT BET AMOUNT ON TOP OF A CHIP */}
                {player["bet"] > 0 ? (
                  <h1 className="text-2xl">{player["bet"]}</h1>
                ) : (
                  <h1 className="text-2xl">0</h1>
                )}
              </div>
              <Betting setPlayer={setPlayer} />
            </>
          ) : (
            // your hand
            <>
              <div
                id="playerHand"
                className="flex justify-center mt-14 relative scale-[0.7] flex-wrap w-[350px]"
              >
                <h1
                  className={`text-3xl text-primary absolute -top-8 -right-10 ${
                    winner == "dealer" ? "line-through" : ""
                  }`}
                >
                  {player["cardSum"]}
                </h1>

                <PlayersHand cards={player != null ? player["cards"] : [""]} />

                {/* This logic might get changed if aces are introduced ie 17/7 */}
                {parseInt(player["cardSum"]) > 21 ? (
                  <h1 className="text-red-500 text-4xl absolute top-0 z-50 animate-getAlert">
                    exceeded
                  </h1>
                ) : null}
                {winner != "" ? (
                  <h1 className="text-primary text-4xl absolute -top-10  z-50 animate-getAlertWinner">
                    {winner} {winner == "tie" ? "" : "win"}{" "}
                    {winner == "you" ? ":)" : ":("}
                  </h1>
                ) : null}
              </div>
              <div className="bg-[url('/src/assets/cards/yourShadow.png')] bg-cover blur-sm opacity-25 bg-no-repeat w-60 h-10 mt-10 relative scale-[0.8] md:scale-100 -z-50"></div>
            </>
          )}

          {/* Game Controls */}
          <div className="mt-14">
            <div className="w-full h-[6px] rounded-full bg-loading opacity-50 mb-4 relative">
              {stopLoading ? (
                <div
                  className={`absolute top-0 bottom-0 rounded-full bg-primary animate-[loading_linear_10s]`}
                ></div>
              ) : (
                ""
              )}
            </div>

            {gameState?.status === 0 ? (
              <Controls handleDecision={handleControlDecision} type="BETTING" />
            ) : (
              <Controls
                handleDecision={handleControlDecision}
                type="PLAYING"
                canPlay={gameState["currentPlayerTurn"] === playerID && canPlay}
              />
            )}
          </div>

          {/* Balance + Round Bet */}
          <div className="absolute right-8 bottom-8 w-64 h-16 border-2 rounded-md flex justify-center flex-col p-4 opacity-50">
            <h1 className="text-2xl">{"WAGER: " + player["bet"]}</h1>
            <h1 className="text-2xl">{"BALANCE: " + player["balance"]}</h1>
          </div>
        </>
      )}
    </div>
  );
}

export default Blackjack;
