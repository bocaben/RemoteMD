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

//// TODO: Add ServerName to config file, then only waitForUSB as 'Robot'.
//getArdPort();
//var waitForUsb = setInterval(getArdPort, checkingTimeInterval);

var ack = { 'status': 'ack' };
var angles = { '1': 20, '2': 45, '3': 90 };
const forwardCms = 10;

// Homepage
router.get('/', function (req, res) {
    res.render('info', { message: 'Available options: /moveXY/x/y, /move/[l|r|f|b], flashlight/[toggle|on|off]' });
});

// Go - [forward|back]
router.all('/go/:direction/', function (req, res) {
    if (!['forward', 'back'].includes(req.params.direction)) {
        res.render('info', { message: 'Available options: turn/[right|left]/[1-3]' });
        return;
    }

    goCms = forwardCms;
    if (req.params.direction === 'back') {
        goCms *= -1;
    }
    console.log(`Got from Cloud to go ${req.params.direction} ${forwardCms} cms`);
    mappingWrite("FO_" + goCms);
    res.json(ack);
});

// Turn - turn/[right|left]/[1-3]
router.all('/turn/:direction/:magnitude', function (req, res) {
    if ((!['right', 'left'].includes(req.params.direction)) ||
        (!['1', '2', '3'].includes(req.params.magnitude))) {
        res.render('info', { message: 'Available options: turn/[right|left]/[1-3]' });
        return;
    }

    degrees = angles[req.params.magnitude];
    if (req.params.direction === 'left') {
        degrees *= -1;
    }
    console.log(`Got from Cloud to turn ${degrees} degrees ${req.params.direction}`);
    mappingWrite("TU_" + degrees);
    res.json(ack);
});

// Flashlight - flashlight/[on|off]
router.all('/flashlight/:toggleValue', function (req, res) {
    if (!['on', 'off'].includes(req.params.toggleValue)) {
        res.render('info', { message: 'Available options: flashlight/[toggle|on|off]' });
        return;
    }

    console.log('Got from Cloud to make flashlight: ' + req.params.toggleValue);
    mappingWrite("FL_" + ((req.params.toggleValue === 'on') ? 1 : 0));
    res.json(ack);
});




function mappingWrite(msg) {
    console.log('Writing to mapping: ' + msg);
    global.child.stdin.write(msg + '\n');
}


module.exports = router;



//function getArdPort() {
//    SerialPort.list(function findArduino(err, ports) {
//        var port = ports.find(function (port) { return ("" + port.manufacturer).startsWith("Arduino"); });
//        if (port == undefined) {
//            //console.log("ROBOT: No Arduino connected!");
//            return;
//        }
//        console.log("ROBOT: Found Arduino - " + port.comName + " [" + port.manufacturer + "]");
//        ardPort = new SerialPort(port.comName, 115200);
//
//        Readline = SerialPort.parsers.Readline; // make instance of Readline parser
//        parser = new Readline(); // make a new parser to read ASCII lines
//        ardPort.pipe(parser); // pipe the serial stream to the parser
//
//        ardPort.on('open', showPortOpen);
//        parser.on('data', readSerialData);
//        ardPort.on('close', showPortClose);
//        ardPort.on('error', showError);
//        clearInterval(waitForUsb);
//    });
//}
//
//function showPortOpen() {
//    console.log('ROBOT: Arduino port open. Data rate: ' + ardPort.baudRate);
//}
//
//function readSerialData(data) {
//    console.log('ROBOT: Got from Arduino: ' + data);
//}
//
//function showPortClose() {
//    console.log('ROBOT: Port closed.');
//    ardPort = undefined;
//    waitForUsb = setInterval(getArdPort, checkingTimeInterval);
//
//}
//
//function showError(error) {
//    console.log('ROBOT: Serial port error: ' + error);
//    //ardPort = undefined;
//    //waitForUsb = setInterval(getArdPort, checkingTimeInterval);
//}
//
//function tryArdWrite_(msg) {
//    if (ardPort != undefined)
//        ardPort.write(msg + '\n');
//    else
//        console.log('ROBOT: No Arduino connected, failed to send msg: "' + msg + '"');
//}