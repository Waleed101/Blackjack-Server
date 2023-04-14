import {FC} from "react"


interface OpponentsHandProps {
  cards: Array<string>;
}


const PlayersHand: FC<PlayersHandProps> =  ({cards}) => {
  {/* need this */}
  return (<div> 
<div className="flex flex-wrap justify center">
{cards.map((card, index) => {
    let cardArr = card.split("");
    const suit = cardArr.pop();
    const cardNo = cardArr.join("");
     
    const sendToAbove = index > 2 ? `absolute sendToAbove -bottom-32` : "";
    let cardPath = `/src/assets/cards/card${suit}.png`
    return (
     
      <div
        key={index}
        style = {{backgroundImage: `url(${cardPath})`}}
        className={`flex justify-center items-center text-6xl text-[#6D5C5C] bg-cover w-[9rem] h-[14rem] -ml-6 animate-getCard -rotate-8 ${sendToAbove}`}
      >
        {cardNo}
        
      </div>
    );
  })}
  </div>
  {/* until here */}



  </div>);
};

export default PlayersHand;
