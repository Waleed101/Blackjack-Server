import { FC } from "react";

interface PingAnimationProps {
  top: string;
  right: string;
}

const PingAnimation: FC<PingAnimationProps> = ({ top, right }) => {
  return (
    <span className={`absolute ${right} ${top} flex h-4 w-4`}>
      <span className="animate-ping absolute inline-flex h-full w-full rounded-full bg-emerald-600 opacity-75"></span>
      <span className="relative inline-flex rounded-full h-4 w-4 bg-emerald-700"></span>
    </span>
  );
};

export default PingAnimation;
