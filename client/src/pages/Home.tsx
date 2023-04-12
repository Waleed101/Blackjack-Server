import playIcon from "../assets/playIcon.png";
import { Link } from "react-router-dom";

export function Home() {
  return (
    <div className="w-screen h-screen flex items-center justify-center flex-col">
      <div className="w-auto">
        <div className="flex flex-col items-center">
          <h2 className="text-4xl font-bold leading-3">PIXEL</h2>
          <h1 className="text-8xl font-extrabold mb-6">BLACKJACK</h1>
          <Link
            className="w-auto flex items-center animate-bounce text-2xl"
            to={"/game"}
          >
            Play now <img className="w-4 h-6 ml-2" src={playIcon} alt="" />
          </Link>
        </div>
      </div>
    </div>
  );
}
