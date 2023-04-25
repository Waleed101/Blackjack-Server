// Importing necessary libraries
import cors from 'cors';
import express from 'express';
import net from 'net';
import bodyParser from 'body-parser'

// Declaring constants; getting PORT for the server from when the command is run (i.e., node index {PORT})
const TIMEOUT = 20; // number of seconds before booting a player
const SERVER_PORT = parseInt(process.argv[2] ?? 2000);

// Keeps track of all the sockets, the last data recieved, and when it was last polled
const sockets = {}

// Setting up the API
const app = express();

app.use(cors());

app.use(bodyParser.json())

// Handles the initial connection request
function initalConnection() {
    // Create a connection to the server on SERVER_PORT
    // Use a promise to wait for the connection to be resolved 
    return new Promise((resolve, reject) => {

        const sock = new net.Socket();

        sock.connect(SERVER_PORT, 'localhost', () => {
            console.log('Connected to server');
        })

        // Returning the initial ID and the reference to the socket
        sock.on('data', (data) => {
            resolve([sock, data.toString()])
        })

        // Error handling
        sock.on('error', (err) => {
            reject(err);
        })
    })
}

// 
app.get("/connect", (req, res) => {
    // socket connection -> send back to the client ID we get back, save it in the dictionary
    // for here... we want it to wait until it gets data back from the server

    console.log("Recieved new connection...")

    // Connect the Server
    const resp = initalConnection()

    resp.then((data) => {
        let [sock, initalData] = data

        // Parse the data
        let procData = JSON.parse(initalData)
        let playerID = procData["playerID"]
        let gameState = procData["gameState"]

        // Create the long term socket management
        manageSocket(sock, playerID)

        // data will hold the broadcast message
        // thats the reference to the socket
        sockets[playerID] = { sock: sock, data: gameState, timestamp: Date.now() }
        
        // Send back the response to the React app
        const resp = {
            id: playerID
        }
        
        res.send(JSON.stringify(resp))
    }).catch((err) => {
        console.log(err)
    })
});

// Polling endpoint
app.get('/update/:id', (req, res) => {
    console.log(`Got request from ${req.params.id}`) // If this is printing, we're getting requests
    sockets[req.params.id].timestamp = Date.now() // Show that the client has recently updated
    res.send(sockets[req.params.id]?.data ?? {}) // Send back the data if its not empty
});

// Endpoint used to perform actions (BETTING, HITTING, STANDING)
app.post('/action/:id', (req, res) => {
    console.log(`Client-${req.params.id} performed an action`)
    sockets[req.params.id]['sock'].write(Buffer.from(JSON.stringify(req.body)))
})

// Start the API
app.listen(3000, () => {
    console.log('Main API port 3000!');
    console.log(`All calls will be routed to server @ ${SERVER_PORT}`)
});

// the player will send their requests at different times from when the Server will send via the socket
// the backend will manage the difference; if it gets data from the socket, it saves it
// if it gets a request from the player, it sends the data it has saved
function manageSocket(sock, id) {

    sock.on('data', (data) => {
        if (sockets[id])
            sockets[id].data = data.toString()
    })

    // When the server closes, set the game state to 3 (0: betting, 1: playing, 2: end of game)
    // React will boot all the players and show a server has closed message
    sock.on('close', () => {
        console.log('Server has closed.');
        Object.keys(sockets).forEach(row => {
            sockets[row].data = {
                status: 3
            }
        })
    })

    sock.on('error', (err) => {
        console.log(`Error: ${err}`)
    })
}

// Function to always check if players are still connected
function checkDisconnectedClients() {
    // It'll iterate through all the sockets and check if the last poll >20 seconds ago
    for (let key in sockets) {
        if (sockets[key]['timestamp'] < Date.now() - TIMEOUT * 1000) {
            // If so, close the socket and delete the player
            sockets[key]['sock'].end()
            console.log("Disconnected Player #" + key)
            delete sockets[key]
        }
    }
}

setInterval(checkDisconnectedClients, 5000)