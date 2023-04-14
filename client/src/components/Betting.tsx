import { Dispatch, FC, SetStateAction } from "react";
import { Player } from "../types/broadcastResponse";

const chips = [1, 5, 10, 25];

interface BettingProps {
  setPlayer: Dispatch<SetStateAction<Player>>;
}

const Betting: FC<BettingProps> = ({ setPlayer }) => {
  const handleBet = (amount: number) => {
    setPlayer((prev) => {
      const newBet = prev["bet"] + amount;
      return { ...prev, bet: newBet };
    });
  };

  return (
    <div className="flex">
      {chips.map((val) => {
        return (
          <div className="flex flex-col place-content-center m-2" key={val}>
            <div
              className={`bg-[url('/src/assets/chips/chip_1.png')] bg-no-repeat bg-cover w-16 h-16 hover:scale-[1.05] cursor-pointer`}
              onClick={() => handleBet(val)}
            ></div>
            <div className="text-center	">{val}</div>
          </div>
        );
      })}
    </div>
  );
};

export default Betting;
