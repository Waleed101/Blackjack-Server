import { Dispatch, FC, SetStateAction } from "react";
import { Player } from "../types/broadcastResponse";

const chips = [1, 5, 10, 25];

interface BettingProps {
  setPlayer: Dispatch<SetStateAction<Player>>;
}

const Betting: FC<BettingProps> = ({ setPlayer }) => {
  const handleBet = (e: HTMLButtonElement) => {
    setPlayer((prev) => {
      const newBet = prev["bet"] + parseInt(e.value);
      return { ...prev, bet: newBet };
    });
  };

  return (
    <>
      <div>
        {chips.map((val) => {
          return (
            <>
              <div
                className={`bg-[url('/src/assets/chips/chip_${val}.png')] bg-inherit bg-no-repeat`}
              >
                <button
                  value={val}
                  onClick={(e) => handleBet(e.currentTarget)}
                ></button>
              </div>
              <h1>{val}</h1>
            </>
          );
        })}
      </div>
    </>
  );
};

export default Betting;
