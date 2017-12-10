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

/*~~~~~~ handle POST requests ~~~~~~*/
router.all('/flashlight/:toggleValue', function (req, res) {
    console.log('CLOUD: Got request for /cloud/flashlight/' + req.params.toggleValue)
    if (['on', 'off'].includes(req.params.toggleValue)) {
        //TODO: Ensure this is async
        fetch(robotURL + '/robot/flashlight/' + req.params.toggleValue, {
            method: 'POST',
            headers: { "Content-type": "application/json" },
            body: JSON.stringify()
        }
        ).then(response => {
            if (response.ok) { return response.json(); }
            throw new Error('Request failed!');
        },
            networkError => console.log(networkError.message)
        ).then(jsonResponse => {
            return jsonResponse;
        });
        res.json({ "flashlight": req.params.toggleValue });
    }
    else
        res.json({ "error": "bad url" });
})

router.post('/move/:turnValue', function (req, res) {
    fetch(robotURL + '/robot/move/' + req.params.turnValue, { 
		method: 'POST', 
		headers: { "Content-type": "application/json"},
		body: JSON.stringify()}
	).then(response => {
   		 if (response.ok) { return response.json(); }
    	 throw new Error('Request failed!'); },
	  networkError => console.log(networkError.message)
		).then(jsonResponse => { 
				return jsonResponse; 
			}); 
})

router.post('/stop', function(req, res) {
    fetch(robotURL + '/robot/stop', { 
		method: 'POST', 
		headers: { "Content-type": "application/json"},
		body: JSON.stringify()}
	).then(response => {
   		 if (response.ok) { return response.json(); }
    	 throw new Error('Request failed!'); },
	  networkError => console.log(networkError.message)
		).then(jsonResponse => { 
				return jsonResponse; 
			}); 
})

router.post('/move', function(req, res) {
    fetch(robotURL + '/robot/move', { 
		method: 'POST', 
		headers: { "Content-type": "application/json"},
		body: JSON.stringify()}
	).then(response => {
   		 if (response.ok) { return response.json(); }
    	 throw new Error('Request failed!'); },
	  networkError => console.log(networkError.message)
		).then(jsonResponse => { 
				return jsonResponse; 
			}); 
})


router.post('/move/right', function(req, res) {
    fetch(robotURL + '/robot/move/right', { 
		method: 'POST', 
		headers: { "Content-type": "application/json"},
		body: JSON.stringify()}
	).then(response => {
   		 if (response.ok) { return response.json(); }
    	 throw new Error('Request failed!'); },
	  networkError => console.log(networkError.message)
		).then(jsonResponse => { 
				return jsonResponse; 
			}); 
})


module.exports = router