import cors from 'cors';
import express from 'express';
import net from 'net';

const SERVER_PORT = 2008;

const sockets = {}

const app = express();

app.use(cors());

function initalConnection() {
    // create a connection to the server on SERVER_PORT

    return new Promise((resolve, reject) => {

        const sock = new net.Socket();

        sock.connect(SERVER_PORT, 'localhost', () => {
            console.log('Connected to server');
        })

        sock.on('data', (data) => {
            console.log(data.toString())
            resolve(sock, data)
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

    resp.then((sock, clientID) => {
        manageSocket(sock, clientID)

        // data will hold the broadcast message
        // thats the reference to the socket

        sockets[clientID] = {sock: sock, data: null}
        console.log(clientID)
        res.send(clientID)
    }).catch((err) => {
        console.log(err)
    })
});

app.get('/update/:id', (req, res) => {
    console.log(`Client-${id} requested an update`)
    res.send(sockets[id])
});

app.listen(3000, () => {
  console.log('Main API port 3000!');
  console.log(`All calls will be routed to server @ ${SERVER_PORT}`)
});

// the player will send their requests at different times from when the Server will send via the socket
// the backend will manage the difference; if it gets data from the socket, it saves it
// if it gets a request from the player, it sends the data it has saved

function manageSocket(sock, id) {
    
    sock.on('data', (data) => {
        sockets[id].data = data
    })

    sock.on('close', () => {
        console.log('Connection closed');
    })

    sock.on('error', (err) => {
        console.log(`Error: ${err}`)
    })
}