import cors from 'cors';
import express from 'express';
import net from 'net';
import bodyParser from 'body-parser'

const TIMEOUT = 20; // number of seconds before booting a player
const SERVER_PORT = parseInt(process.argv[2] ?? 2000);

const sockets = {}

const app = express();

app.use(cors());

app.use(bodyParser.json())

function initalConnection() {
    // create a connection to the server on SERVER_PORT

    return new Promise((resolve, reject) => {

        const sock = new net.Socket();

        sock.connect(SERVER_PORT, 'localhost', () => {
            console.log('Connected to server');
        })

        sock.on('data', (data) => {
            // console.log(data.toString())
            resolve([sock, data.toString()])
        })

        sock.on('error', (err) => {
            reject(err);
        })
    })
}

app.get("/connect", (req, res) => {
    // socket connection -> send back to the client ID we get back, save it in the dictionary
    // for here... we want it to wait until it gets data back from the server

    console.log("Recieved new connection...")

    const resp = initalConnection()

    resp.then((data) => {
        let [sock, initalData] = data

        if (initalData == "0") {
            console.log("Client denied in joining a full table")
            res.send("Full table")
        } else {
            let procData = JSON.parse(initalData)
            let playerID = procData["playerID"]
            let gameState = procData["gameState"]

            manageSocket(sock, playerID)

            // data will hold the broadcast message
            // thats the reference to the socket
            sockets[playerID] = { sock: sock, data: gameState, timestamp: Date.now() }
            console.log(playerID)
            console.log(sockets)
            const resp = {
                id: playerID
            }
            console.log(resp)
            res.send(JSON.stringify(resp))
        }
    }).catch((err) => {
        console.log(err)
    })
});

app.get('/update/:id', (req, res) => {
    if(sockets[req.params.id]?.data) {
        console.log("Good data.")
    } else {
        console.log("Bad Data")
    }
    sockets[req.params.id].timestamp = Date.now()
    res.send(sockets[req.params.id]?.data ?? {})
});

app.post('/action/:id', (req, res) => {
    console.log(`Client-${req.params.id} performed an action`)
    console.log(req.body)
    sockets[req.params.id]['sock'].write(Buffer.from(JSON.stringify(req.body)))
})

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

function checkDisconnectedClients() {
    for (let key in sockets) {
        if (sockets[key]['timestamp'] < Date.now() - TIMEOUT * 1000) {
            sockets[key]['sock'].end()
            console.log("Disconnected the thingy")
            delete sockets[key]
        }
    }
}

setInterval(checkDisconnectedClients, 5000)