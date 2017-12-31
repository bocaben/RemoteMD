var express = require('express');
var router = express.Router();
var fetch = require('node-fetch');


/*~~~~~~ Serving static files ~~~~~~*/
router.use(express.static(__dirname + '../views'));


/*~~~~~~ handle GET requests ~~~~~~*/
/*
router.get('/', function(req, res) {
  res.sendFile(__dirname + '../views/' + 'index.html');
})

router.get('/doctor', function(req, res) {
  res.sendFile(__dirname + '../views/' + 'doctor-index.html');
})

router.get('/patient', function(req, res) {
  res.sendFile(__dirname + '../views/' + 'doctor-index.html');
})


router.get('/navigation', function(req, res) {
  res.sendFile(__dirname + '../views/' + 'navigation.html');
})
*/

//var robotURL = 'http://192.168.43.77' + ':8080';
var robotURL = 'http://localhost' + ':8080';



function genericFetch(url) {
	fetch(url, {
		method: 'POST',
		headers: { "Content-type": "application/json" },
		body: JSON.stringify()
	}).then(response => {
			if (response.ok) { return response.json(); }
			throw new Error('Request failed!');
		}, networkError => console.log(networkError.message)
		).then(jsonResponse => { return jsonResponse; });
}

/*~~~~~~ handle POST requests from the browser and send POST to the Robot ~~~~~~*/

router.post('/flashlight/:toggleValue', function (req, res) {
    console.log('CLOUD: Got request for /cloud/flashlight/' + req.params.toggleValue);
    if (['on', 'off'].includes(req.params.toggleValue)) {
			//TODO: Ensure this is async
			let url = robotURL + '/robot/flashlight/' + req.params.toggleValue;
			genericFetch(url);
			
			res.json({ "flashlight": req.params.toggleValue });
    } else res.json({ "error": "bad url" });
})

router.post('/turn/left/:turnValue', function (req, res) {
		console.log('CLOUD: Got request for /cloud/turn/left/' + req.params.turnValue);
		let url = robotURL + '/robot/turn/left/' + req.params.turnValue;
		genericFetch(url);
})

router.post('/turn/right/:turnValue', function (req, res) {
	console.log('CLOUD: Got request for /cloud/turn/right/' + req.params.turnValue);
	let url = robotURL + '/robot/turn/right/' + req.params.turnValue;
	genericFetch(url);
})


router.post('/go/forward', function(req, res) {
		console.log('CLOUD: Got request for /cloud/go/forward/');
		let url = robotURL + '/robot/go/forward';
		genericFetch(url);
})

router.post('/go/back', function(req, res) {
	console.log('CLOUD: Got request for /cloud/go/back/');
	let url = robotURL + '/robot/go/back';
	genericFetch(url);
})


/*~~~~~~ handle POST requests from the Robot and send to the browser ~~~~~~*/

router.get('/conf/:msg', function (req, res) {
	console.log('CLOUD: Got confirm from robot for /cloud/' + req.params.msg);
	
})



module.exports = router