var express = require('express');
var app = express();

/*~~~~~~ Serving static files ~~~~~~*/
app.use(express.static(__dirname + '../views'));


/*~~~~~~ handle GET requests ~~~~~~*/
/*
app.get('/', function(req, res) {
  res.sendFile(__dirname + '../views/' + 'index.html');
})

app.get('/doctor', function(req, res) {
  res.sendFile(__dirname + '../views/' + 'doctor-index.html');
})

app.get('/patient', function(req, res) {
  res.sendFile(__dirname + '../views/' + 'doctor-index.html');
})


app.get('/navigation', function(req, res) {
  res.sendFile(__dirname + '../views/' + 'navigation.html');
})
*/

/*~~~~~~ handle POST requests ~~~~~~*/
app.post('/cloud/flashlight/on', function(req, res) {
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

app.post('/cloud/flashlight/off', function(req, res) {
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

app.post('/cloud/move/left', function(req, res) {
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

app.post('/cloud/stop', function(req, res) {
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

app.post('/cloud/move', function(req, res) {
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


app.post('/cloud/move/right', function(req, res) {
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


