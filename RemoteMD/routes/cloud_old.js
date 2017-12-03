// Rest API for Robot->Cloud

'use strict';
var express = require('express');
var router = express.Router();

/* GET home page. */
router.get('/', function (req, res) {
    res.render('index', { title: 'Express' });
});

router.get('/what', function (req, res) {
    res.render('Login', { someVar: 'some Var' });
});

module.exports = router;


function sendJson(url, data) {
    // Sending and receiving data in JSON format using POST method
    var xhr = new XMLHttpRequest();
    xhr.open("POST", url, true);
    xhr.setRequestHeader("Content-type", "application/json");
    xhr.onreadystatechange = function () {
        if (xhr.readyState === 4 && xhr.status === 200) {
            var json = JSON.parse(xhr.responseText);
            console.log(json.email + ", " + json.password);
        }
    };
    xhr.send(data.stringify);
}
