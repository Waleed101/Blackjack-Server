# Online Blackjack Server

<p align="center">
  <img src="https://github.com/Waleed101/OS-Group-Project/blob/main/static/ezgif.com-video-to-gif.gif" />
</p>

### Goal: Create a multi-threaded application for online blackjack that allows multiple players to play concurrently, ensuring fairness and synchronization by using semaphores to regulate access to the game state.

## Technologies & Languages:
- C++
- JavaScript & TypeScript
- Node.js
- Docker
- React.js

## Key Learnings

Connecting the C++ server to the React front end was an obstacle that the team had to
overcome. Due to React’s inability to establish a TCP socket with the server, the team needed an
unorthodox solution to allow the client and server to communicate properly. We decided to
implement a node js express middleware that receives the HTTP requests sent from the React
frontend, and depending on the HTTP data writes relevant data to the appropriate socket that is
connected with the C++ server. The server would then receive these packets from the express
API and update the game state accordingly. For every game update, the server broadcasts the
game state to each active socket which is temporarily stored in the express middleware and
relayed to the React frontend for every GET request. Our team also learned how to use a vector
object to keep track of active threads. This allowed for practical referencing to active threads for
easy garbage collection and management. Finally, our team learned how to make our application
both multi-transactional and multi-user. This was a combination of the key learnings that we took
away from previous labs that allowed multiple players to play at the same table and also have
multiple tables running when there are no more seats left at a table.

## Design

### Synchronization of Objects
### Thread Management
### Termination

## In-Game Screenshots

<p align="center">
  <img src="https://github.com/Waleed101/OS-Group-Project/blob/main/static/first.png" width="50%" alt="First loading screen..."/>
  <br /><i>First loading screen</i>
</p>
<p align="center">
  <img src="https://github.com/Waleed101/OS-Group-Project/blob/main/static/betting.png" width="50%" alt="Betting screen..."/>
  <br /><i>Betting screen</i>
</p>
<p align="center">
  <img src="https://github.com/Waleed101/OS-Group-Project/blob/main/static/turn_indiv.png" width="50%" alt="Individual turn..."/>
  <br /><i>Individual turn</i>
</p>
<p align="center">
  <img src="https://github.com/Waleed101/OS-Group-Project/blob/main/static/winning.png" width="50%" alt="Player winning..."/>
  <br /><i>Winning</i>
</p>
<p align="center">
  <img src="https://github.com/Waleed101/OS-Group-Project/blob/main/static/multiplayer.png" width="50%" alt="Multiplayer.png"/>
  <br /><i>Multiplayer</i>
</p>
