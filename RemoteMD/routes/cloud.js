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

var robotURL = 'http://192.168.43.77' + ':8080'
//var robotURL = 'http://localhost' + ':8080'



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

router.all('/flashlight/:toggleValue', function (req, res) {
    console.log('CLOUD: Got request for /cloud/flashlight/' + req.params.toggleValue);
    if (['on', 'off'].includes(req.params.toggleValue)) {
			//TODO: Ensure this is async
			let url = robotURL + '/robot/flashlight/' + req.params.toggleValue;
			genericFetch(url);
			
			res.json({ "flashlight": req.params.toggleValue });
    } else res.json({ "error": "bad url" });
})

router.post('/move/:turnValue', function (req, res) {
		console.log('CLOUD: Got request for /cloud/move/' + req.params.turnValue);
		let url = robotURL + '/robot/move/' + req.params.turnValue;
		genericFetch(url);
})

router.post('/stop', function(req, res) {
		console.log('CLOUD: Got request for /cloud/stop/');
		let url = robotURL + '/robot/stop';
		genericFetch(url);
})

router.post('/move', function(req, res) {
		console.log('CLOUD: Got request for /cloud/move/');
		let url = robotURL + '/robot/move';
		genericFetch(url);
})


/*~~~~~~ handle POST requests from the Robot and send POST to the browser ~~~~~~*/

router.all('/confirm/flashlight/:toggleValue', function (req, res) {
	console.log('CLOUD: Got confirm for /cloud/flashlight/' + req.params.toggleValue);
	if (['on', 'off'].includes(req.params.toggleValue)) {
		//TODO: Ensure this is async
		let url = robotURL + '/robot/flashlight/' + req.params.toggleValue;
		genericFetch(url);
		
		res.json({ "flashlight": req.params.toggleValue });
	} else res.json({ "error": "bad url" });
})

router.post('/confirm/move/:turnValue', function (req, res) {
	console.log('CLOUD: Got confirm for /cloud/move/' + req.params.turnValue);
	let url = robotURL + '/robot/move/' + req.params.turnValue;
	genericFetch(url);
})

router.post('/confirm/stop', function(req, res) {
	console.log('CLOUD: Got confirm for /cloud/stop/');
	let url = robotURL + '/robot/stop';
	genericFetch(url);
})

router.post('/confirm/move', function(req, res) {
	console.log('CLOUD: Got confirm for /cloud/move/');
	let url = robotURL + '/robot/move';
	genericFetch(url);
})


module.exports = router