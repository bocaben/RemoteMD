// Rest API for Cloud->Robot

'use strict';
var express = require('express');
var router = express.Router();
var SerialPort = require('serialport');

var ardPort;
var Readline;
var parser;
var ardPortOpen = false;

var checkingTimeInterval = 5000;

// TODO: Add ServerName to config file, then only waitForUSB as 'Robot'.
getArdPort();
var waitForUsb = setInterval(getArdPort, checkingTimeInterval);


// Homepage
router.get('/', function (req, res) {
    res.render('info', { message: 'Available options: /moveXY/x/y, /move/[l|r|f|b], flashlight/[toggle|on|off]' });
});

// Flashlight - flashlight/[toggle|on|off]
// TODO: switch to .post for production
router.all('/flashlight/:toggleValue', function (req, res) {
    if (['on','off','toggle'].includes(req.params.toggleValue))
    {
        res.json({ "flashlight": req.params.toggleValue});
        console.log('ROBOT: Got from Cloud to make flashlight: ' + req.params.toggleValue);
        tryArdWrite("flashlight " + req.params.toggleValue);
    }
    else
        res.render('info', { message: 'Available options: flashlight/[toggle|on|off]'});
});

module.exports = router;

function getArdPort() {
    SerialPort.list(function findArduino(err, ports) {
        var port = ports.find(function (port) { return ("" + port.manufacturer).startsWith("Arduino"); });
        if (port == undefined) {
            console.log("ROBOT: No Arduino connected!");
            return;
        }
        console.log("ROBOT: Found Arduino - " + port.comName + " [" + port.manufacturer + "]");
        ardPort = new SerialPort(port.comName, 9600);

        Readline = SerialPort.parsers.Readline; // make instance of Readline parser
        parser = new Readline(); // make a new parser to read ASCII lines
        ardPort.pipe(parser); // pipe the serial stream to the parser

        ardPort.on('open', showPortOpen);
        parser.on('data', readSerialData);
        ardPort.on('close', showPortClose);
        ardPort.on('error', showError);
        clearInterval(waitForUsb);
    });
}

function showPortOpen() {
    console.log('ROBOT: Arduino port open. Data rate: ' + ardPort.baudRate);
}

function readSerialData(data) {
    console.log('ROBOT: Got from Arduino: ' + data);
}

function showPortClose() {
    console.log('ROBOT: Port closed.');
    ardPort = undefined;
    waitForUsb = setInterval(getArdPort, checkingTimeInterval);

}

function showError(error) {
    console.log('ROBOT: Serial port error: ' + error);
    //ardPort = undefined;
    //waitForUsb = setInterval(getArdPort, checkingTimeInterval);
}

function tryArdWrite(msg) {
    if (ardPort != undefined)
        ardPort.write(msg + '\n');
    else
        console.log('ROBOT: No Arduino connected, failed to send msg: "' + msg + '"');
}