import { FC } from "react";

interface OpponentsHandProps {
  cards: Array<string>;
}

const OpponentsHand: FC<OpponentsHandProps> = ({ cards }) => {
  return (
    <div className="flex scale-50">
      {cards.map((card, index) => {
        let cardArr = card.split("");
        const suit = cardArr.pop();
        const cardNo = cardArr.join("");

        const sendToAbove = index > 2 ? `absolute sendToAbove -bottom-32` : "";
        let cardPath = `/src/assets/cards/card${suit}.png`;
        return (
          <div
            key={index}
            style={{ backgroundImage: `url(${cardPath})` }}
            className={`flex justify-center items-center text-6xl text-[#6D5C5C] w-[7rem] h-[11rem] bg-cover -ml-6 animate-getCard -rotate-2 ${sendToAbove}`}
          >
            {cardNo}
          </div>
        );
      })}
    </div>
  );
};

export default OpponentsHand;
