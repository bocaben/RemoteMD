var express = require('express');
var http = require('http');
var WebSocket = require('ws');
var app = express();

var server = http.createServer(app);
const wss_browser = new WebSocket.Server({ server : server});
const ws_robot = new WebSocket('ws://localhost:8080');


var routes = require('./routes/index');

app.use('/', routes);

ws_robot.onopen = function() {
	console.log('connecting with robot..');
}


wss_browser.on('connection', function connection(ws, req) {
  console.log('connection with browser: established');

  // forwarding messages from browser to robot
  ws.on('message', function incoming(data) {
    console.log('got msg from browser: ' + data);

    ws_robot.send(data);
  }); 

  // forwarding messages from robot to browser
  ws_robot.onmessage = function(msg) {
    var data = msg.data;
    console.log('got msg from robot: ' + data);

    ws.send(data)
  }
});



wss_browser.on('close', function() {
  console.log('Closing connection');
});

server.listen(80).on('error', console.log);
