var express = require('express');
var router = express.Router();

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

/*~~~~~~ handle POST requests ~~~~~~*/
router.post('/cloud/flashlight/on', function(req, res) {
  fetch('ip/robot/flashlight/on', { 
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

router.post('/cloud/flashlight/off', function(req, res) {
  fetch('/robot/flashlight/off', { 
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

router.post('/cloud/move/left', function(req, res) {
  fetch('ip/robot/move/left', { 
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

router.post('/cloud/stop', function(req, res) {
  fetch('ip/robot/stop', { 
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

router.post('/cloud/move', function(req, res) {
  fetch('ip/robot/move', { 
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


router.post('/cloud/move/right', function(req, res) {
  fetch('ip/robot/move/right', { 
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