// Rest API for Cloud->Robot

'use strict';
var express = require('express');
var router = express.Router();

// Homepage
router.get('/', function (req, res) {
    res.render('info', { message: 'Available options: /moveXY/x/y, /move/[l|r|f|b], flashlight/[toggle|on|off]' });
});

// Flashlight - flashlight/[toggle|on|off]
// TODO: switch to .post for production
router.all('/flashlight/:toggleValue', function (req, res) {
    if (['on','off','toggle'].includes(req.params.toggleValue))
    {
        res.json({ "flashlight post": req.params.toggleValue });
        console.log('ROBOT: Got request to make flashlight: ' + req.params.toggleValue);

    }
    else
        res.render('info', { message: 'Available options: flashlight/[toggle|on|off]', newtest: 'blah'});
});

//// TODO: switch to .post for production
//router.all('/flashlight/:toggleValue', function (req, res) {
//    if (['on', 'off', 'toggle'].includes(req.params.toggleValue)) {
//        res.json({ "flashlight": req.params.toggleValue });
//        console.log('ROBOT: Got request to make flashlight: ' + req.params.toggleValue);
//    }
//    else
//        res.render('info', { message: 'Available options: flashlight/[toggle|on|off]' });
//});

module.exports = router;
