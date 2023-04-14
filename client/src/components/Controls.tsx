import { FC } from "react";

interface PlayingControlProps {
  handleDecision: Function;
  type: string;
  canPlay?: boolean;
}

const PlayingControls: FC<PlayingControlProps> = ({
  handleDecision,
  type,
  canPlay = false,
}) => {
  return (
    <div>
      {type === "BETTING" ? (
        <button
          onClick={() => handleDecision("CLEAR")}
          className={`text-3xl w-44 h-14 rounded-md bg-primary text-bg transition-colors`}
        >
          clear
        </button>
      ) : (
        <>
          <button
            onClick={() => handleDecision("HIT")}
            disabled={!canPlay}
            className={`text-3xl w-44 h-14 rounded-md ${
              canPlay ? "bg-primary text-bg" : "bg-shadow text-secondary"
            } transition-colors`}
          >
            hit
          </button>
          <button
            onClick={() => handleDecision("STAND")}
            disabled={!canPlay}
            className={`text-3xl w-20 h-14 ${
              canPlay ? "bg-b-secondary text-bg" : "bg-shadow text-secondary"
            } transition-colors rounded-md ml-4`}
          >
            stay
          </button>
        </>
      )}
    </div>
  );
};

export default PlayingControls;
