import { useEffect, useState, useRef } from "react";
import { useInterval } from "../hooks/useInterval";
import { URL, POLL_REFRESH_INTERVAL } from "../constants/constants";
import axios from "axios";

//types
import { Broadcast, Player, Game, Dealer } from "../types/broadcastResponse";

// components
import Controls from "../components/Controls";
import Betting from "../components/Betting";
import OpponentsHand from "../components/OpponentsHand";
import PingAnimation from "../components/PingAnimation";

function Blackjack() {
  // is player connected to the server
  const [isConnected, setIsConnected] = useState(false);
  const [isLoaded, setIsLoaded] = useState(false);
  const [hasPolled, setHasPolled] = useState(false)

  // the game related state
  const [gameState, setGameState] = useState<Game>({
    status: 0,
    timeRemaining: -1,
    hasDealerBusted: false,
    currentPlayerTurn: 4,
    gameID: -1,
  });
  const timeRemainingRef = useRef(gameState['timeRemaining']);

  // player's state
  const [playerID, setPlayerID] = useState(-1);
  const playerIdRef = useRef(playerID);
  const [player, setPlayer] = useState<Player>({
    bet: 0,
    seat: 4, // a seat above the allowable
    cards: [""],
    balance: 200,
    isBusted: false,
    isActive: 1,
    cardSum: "0",
    hasWon: 0
  });
  const betRef = useRef(player['bet'])
  betRef.current = player['bet']

  // dealer
  const [dealer, setDealer] = useState<Dealer>({
    dealerCards: ["", ""],
    cardSum: "0",
  });

  // seats
  const [seats, setSeats] = useState([-1, -1, -1, -1]);

  // timer for move
  const [action, setAction] = useState("STAND");
  const actionRef = useRef(action)
  actionRef.current = action
  const [canPlay, setCanPlay] = useState(true);

  // loading switch
  const [isInTimeout, setInTimeout] = useState(false);

  // timeoutFunction
  let timer: NodeJS.Timer;

  // define a winner
  const [winner, setWinner] = useState<"dealer" | "you" | "tie" | "">("");

  // bet handler
  const handleControlDecision = (choice: string) => {
    // clear button -> amount == 0
    if (choice === "CLEAR") {
      setPlayer((prev) => {
        return { ...player, balance: prev["balance"] + prev["bet"], bet: 0 };
      });
    } else {
      // handle player actions
      setCanPlay(false);
      setAction(choice);
    }
  };

  // server polling for game updates

  useInterval(async () => {
    if (!isConnected || playerID === -1) {
      return;
    }

    const data = await axios.get(`${URL}/update/${playerID}`);
    const gameUpdate: Broadcast = data.data;

    timeRemainingRef.current = gameUpdate['timeRemaining']

    // search through players an assign seats
    const otherPlayers: { [key: number]: Player } = {};
    const tempSeats = [-1, -1, -1, -1];
    let i = 0;  
    for (let pKey in gameUpdate["players"]) {
      tempSeats[i] = i; // map the seats to [0, 1, 2, 3]
      if (parseInt(pKey) !== playerID) {
        otherPlayers[i] = gameUpdate["players"][pKey];
      }
      i++;
    }

    // update dealer states
    setDealer({
      dealerCards: gameUpdate["dealerCards"],
      cardSum: gameUpdate["dealerSum"],
    });

    // update player states
    console.log(gameUpdate["players"])
    const playerRef = gameUpdate["players"][playerID];
    if (gameUpdate["status"] === 0) {
      playerRef["balance"] = player["balance"];
      playerRef["bet"] = player["bet"];
    }
    setPlayer(playerRef);

    // update game states
    let tempGameState = {
      status: gameUpdate["status"],
      timeRemaining: gameUpdate["timeRemaining"],
      hasDealerBusted: gameUpdate["hasDealerBusted"],
      currentPlayerTurn: gameUpdate["currentPlayerTurn"],
      gameID: gameUpdate["gameID"],
    };

    if (Object.keys(otherPlayers).length > 0) {
      tempGameState = { ...tempGameState, ...{ otherPlayers } };
    }
    setGameState(tempGameState);

    // update the seats
    setSeats(tempSeats);

    setIsLoaded(true);

    if (gameUpdate['status'] === 2){
      if (playerRef['hasWon'] === 0){
        setWinner('dealer')
      } else if (playerRef['hasWon'] === 1){
        setWinner('you')
      }else {
        setWinner('tie')
      }
    } else {
      setWinner('')
    }

    if (!hasPolled){
      setHasPolled(true)
    }
  }, POLL_REFRESH_INTERVAL);

  // connect to game, fetch player id
  const initialConnection = async () => {
    const connectionData = await axios.get(URL + "/connect");
    console.log(connectionData.data);
    playerIdRef.current = connectionData.data.id;
    setPlayerID(connectionData.data.id);
  };

  useEffect(() => {
    if (gameState["status"] === 0 && !isInTimeout && hasPolled && timeRemainingRef.current > 1) {
      setInTimeout(true);
      timer = setTimeout(async () => {
        setInTimeout(false);
        setCanPlay(true);
        // make the call at the end of the turn
        await axios.post(URL + `/action/${playerIdRef.current}`, {
          type: "BET",
          betAmount: betRef.current,
        });
      }, (timeRemainingRef.current - 1) * 1000);
    }
  }, [gameState["status"], hasPolled, gameState['timeRemaining']]);

  // controls the timer during playing state
  useEffect(() => {
    if (gameState["status"] === 1 && gameState["currentPlayerTurn"] === player['seat'] && !isInTimeout && timeRemainingRef.current > 1) {
      // start the timer and wait till it finishes to set next actions
      setInTimeout(true);
      timer = setTimeout(async () => {
        setInTimeout(false);
        setCanPlay(true); // allow player to hit/stand again after timer

        // make the call at the end of the turn
        await axios.post(URL + `/action/${playerID}`, {
          type: "TURN",
          action: actionRef.current,
        });
      }, (timeRemainingRef.current - 1) * 1000);
    }
  }, [player["cardSum"], gameState["currentPlayerTurn"]]);

  useEffect(() => {
    setIsConnected(false);
    initialConnection();
    setIsConnected(true);
  }, []);

  return (
    <div className="w-96 max-w-sm h-screen flex items-center flex-col">
      {!isConnected && !isLoaded ? (
        <div>connecting to the table</div>
      ) : (
        <>
        {gameState['status'] === 3 ? <>server closed</> :  <>
        <div className="absolute left-2 top-2 opacity-25 text-2xl">
          Table: {gameState["gameID"]} <br></br>
          Time Remaining: {gameState['timeRemaining']}
        </div>
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
                className={`text-3xl text-primary absolute -top-8 -right-0 ${
                  winner == "you" ? "line-through" : ""
                } `}
              >
                {/* more logic can go here to strike through value when busted */}
                {gameState['status'] === 2 ? dealer["cardSum"] : '---'}
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
          {seats.map((seatIndex, index) => {
            // define extra ping attribute
            if (seatIndex === player["seat"]) {
              return (
                // work here to add some nice background, maybe darken with text & cross?
                <>
                  <div
                    key={index}
                    className="spot w-20 h-32 border-solid border-slate-100 rounded-md border-4 opacity-75 bg-slate-800 text-white flex justify-center items-center"
                  >
                    {seatIndex === gameState["currentPlayerTurn"] ? (
                      <PingAnimation top={"-top-6"} right={"right-[40%]"} />
                    ) : null}
                    YOUR SPOT
                  </div>
                </>
              );
            } else if (seatIndex !== -1) {
              // pass the opponents pId and associated cards here!
              return (
                <>
                  {/* this is where the opponents hand is rendered it needs the styling  */}
                  <div className={`spot`} key={index}>
                    {seatIndex === gameState["currentPlayerTurn"] ? (
                      <PingAnimation top={"top-4"} right={"right-[50%]"} />
                    ) : null}
                    {gameState['status'] === 0 ? <div className="w-20 h-32 border-solid border-slate-100 rounded-md border-4 opacity-75 bg-slate-800 text-white flex justify-center items-center">OPPONENTS</div> :  <OpponentsHand
                      cards={
                        gameState.otherPlayers
                          ? gameState.otherPlayers[seatIndex]["cards"]
                          : [""]
                      }
                    />}
                   
                  </div>
                </>
              );
            } else if (seatIndex === -1) {
              return (
                <div
                  className={`spot w-20 h-32 border-solid border-slate-100 rounded-md border-4 opacity-50`}
                ></div>
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
              className="flex justify-center mt-14 relative scale-75 flex-wrap w-[350px]"
            >
              <h1
                className={`text-3xl text-primary absolute -top-8 -right-0 ${
                  winner == "dealer" ? "line-through" : ""
                }`}
              >
                {player["cardSum"]}
              </h1>

              {player['isActive'] === 1 ? <div className="flex justify-center items-center text-2xl">JOINNING NEXT ROUND...</div> : <>              {player["cards"].map((card, index) => {
                let cardArr = card.split("");
                const suit = cardArr.pop();
                const cardNo = cardArr.join("");

                const sendToAbove =
                  index > 2 ? `absolute sendToAbove -bottom-32` : "";
                let cardPath = `/src/assets/cards/card${suit}.png`;
                return (
                  <div
                    key={index}
                    style={{ backgroundImage: `url(${cardPath})` }}
                    className={`flex justify-center items-center text-6xl text-[#6D5C5C] bg-cover w-[7rem] h-[11rem] -ml-6 animate-getCard -rotate-2 ${sendToAbove}`}
                  >
                    {cardNo}
                  </div>
                );
              })}</>}


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
            {/* {stopLoading ? (
              <div
                className={`absolute top-0 bottom-0 rounded-full bg-primary animate-[loading_linear_10s]`}
              ></div>
            ) : (
              ""
            )} */}
          </div>

          {gameState?.status === 0 ? (
            <Controls handleDecision={handleControlDecision} type="BETTING" />
          ) : (
            <Controls
              handleDecision={handleControlDecision}
              type="PLAYING"
              canPlay={
                gameState["currentPlayerTurn"] === player["seat"] && canPlay && player['isActive'] === 0
              }
            />
          )}
        </div>

        {/* Balance + Round Bet */}
        <div className="absolute right-8 bottom-8 w-64 h-16 border-2 rounded-md flex justify-center flex-col p-4 opacity-50">
          <h1 className="text-2xl">{"WAGER: " + player["bet"]}</h1>
          <h1 className="text-2xl">{"BALANCE: " + player["balance"]}</h1>
        </div>
      </>}
       </>
      )}
    </div>
  );
}

export default Blackjack;
