import { Route, Routes } from "react-router-dom";
import Blackjack from "./pages/Blackjack";
import { Home } from "./pages/Home";

export function Router() {
  return (
    <Routes>
      <Route path="/" element={<Home />} />
      <Route path="/game" element={<Blackjack />} />
    </Routes>
  );
}
