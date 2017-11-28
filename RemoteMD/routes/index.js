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
