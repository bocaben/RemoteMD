var express = require('express');
var http = require('http');
var WebSocket = require('ws');
var fs = require('fs');
var app = express();

var server = http.createServer(app);
const wss_browser = new WebSocket.Server({ server : server});

var routes = require('./routes/index');
app.use('/', routes);

// creating log file
var date = new Date();
var day = date.getDate(); 
var month = date.getMonth() + 1; 
var year = date.getFullYear(); 
var log_name = year  + '-' + month + '-' + day + '.log';
console.log(log_name);
var file = fs.createWriteStream(log_name, { flags: 'a'});

function writeToLog(data) {
  console.log(data);
  var date = new Date();
  var hour = date.getHours(); 
  var minutes = date.getMinutes(); 
  var seconds = date.getSeconds(); 
  var time = hour  + ':' + minutes + ':' + seconds + ': ';
  file.write(time + data + '\n');
} 

// only in Navigation page
wss_browser.on('connection', function connection(ws, req) {
  writeToLog('Connection to Browser: ESTABLISHED');

  //const ws_robot = new WebSocket('ws://132.68.40.73:8080');
  const ws_robot = new WebSocket('ws://localhost:8080');

  ws_robot.onopen = function() {
    writeToLog('Connection to Robot: ESTABLISHED');
    ws.send(JSON.stringify({"robot_conn": "established"}));
  }
  
  ws_robot.onerror = function(error) {
    writeToLog('Connection to Robot: ERROR');
    ws.send(JSON.stringify({"robot_conn": "error"}));
  }
  
  ws_robot.onclose = function() {
    writeToLog('Connection to Robot: CLOSED');
    ws.send(JSON.stringify({"robot_conn": "closed"}));
  }


  // forwarding messages from robot to browser
  ws_robot.onmessage = function(msg) {
    var data = msg.data;
    writeToLog('Message from Robot: ' + data);
    // send message to browser
    ws.send(data)
  }

  // forwarding messages from browser to robot
  ws.on('message', function incoming(data) {
      writeToLog('Message from Browser: ' + data);
      // send message to robot
      ws_robot.send(data);
  }); 

  ws.on('error', function(error) {
    writeToLog('Connection to Browser: ERROR');
    // closing log file
    file.end();
  });

  ws.on('close', function() {
    writeToLog('Connection to Browser: CLOSED');
    // closing log file
    file.end();
  });
});


wss_browser.on('error', function(err) {
  writeToLog('Cloud Server: ERROR');
  // closing log file
  file.end();
});


server.listen(80).on('error', function() {
  writeToLog('Listening: ERROR');
  // closing log file
  file.end();
});


