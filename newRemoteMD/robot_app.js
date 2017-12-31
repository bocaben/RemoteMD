'use strict'
var express = require('express');
var WebSocket = require('ws');
var app = express();


//var server = app.listen(8080).on('error', console.log);
const spawn = require('child_process').spawn;
const mapping = spawn('./mapping');

const wss = new WebSocket.Server({ port : 8080 });

wss.on('connection', function connection(ws, req) {
  console.log('connection with cloud: established');

  ws.on('message', function incoming(data) {
    var msg =  JSON.parse(data);
    console.log('got msg from cloud: ' + data);
    console.log('command: ' + msg.command);
    mapping.stdin.write(msg.command + '\n');
  });

  mapping.stdout.on('data', function(data) {
    console.log('stdout: ' + data);
    ws.send(JSON.stringify({"conf": data.toString()}));
  });
});





wss.on('close', function() {
  console.log('Closing connection with cloud');
});
