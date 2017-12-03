var express = require('express');
var router = express.Router();
var app = express();

/* GET home page. */
router.use('/', express.static(__dirname + '/../views'));

router.get('/', function (req, res) {
    res.sendFile('index.html');
});


module.exports = router;
