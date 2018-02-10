'use strict'
var express = require('express');
var WebSocket = require('ws');
var app = express();

const spawn = require('child_process').spawn;
const mapping = spawn('./MAPING/Debug/MAPING.exe');
//const mapping = spawn('./mapping');

// commands accpeted via console or via write()
process.stdin.pipe(mapping.stdin);

const wss = new WebSocket.Server({ port : 80 });

wss.on('connection', function connection(ws, req) {
  console.log('Connection to Cloud: ESTABLISHED');

  ws.on('message', function incoming(data) {  
    var msg =  JSON.parse(data);
    console.log('Message from Cloud: ' + data);
    console.log('command: ' + msg.command);
    mapping.stdin.write(msg.command + '\n');  
  });

  ws.on('error', function(error) {
    console.log('Connection to Cloud: ERROR');
  });
  
  ws.on('close', function() {
    console.log('Connection to Cloud: CLOSED');
  });

  mapping.stdout.on('data', function(data) {
    console.log('stdout: ' + data);

    ws.send(JSON.stringify({"conf": data.toString()}));
  });
});

wss.on('error', function(error) {
  console.log('Robot Server: ERROR');
});