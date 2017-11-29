'use strict';
var express = require('express');
var router = express.Router();
var path = require('path');

/* GET home page. */
router.get('/', function (req, res) {
    res.render('index.html');
});

router.get('/what', function (req, res) {
    res.render('Login', { someVar: 'some Var' });
});

module.exports = router;
