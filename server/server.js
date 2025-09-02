const { createServer } = require("http");
const { Server } = require("socket.io");

const httpServer = createServer();
const socket = new Server(httpServer, {});

socket.on("connection", (socket) => {
    console.log("socket");
});

httpServer.listen(3001, () => {
    console.log("listening on *:3001");
});