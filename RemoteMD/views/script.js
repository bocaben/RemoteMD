const ws = new WebSocket('ws://localhost/');
//const ws = new WebSocket('ws://35.197.216.190/');


// Navigation's buttons functions
function updateState(str) {
	document.getElementById('p')[0].innerHTML(str);
}

function sendToCloud(command) {
	ws.send(JSON.stringify({"command": command}));
}

function flashlight() {
	console.log('flashlight');

	let buttonClass = document.getElementById('flash').classList;
	let command = 'FL_0';
	let state = 'OFF';

	if (toggleColor('flash', 'on')) {
		command = 'FL_1';
		state = 'ON';
	}
	
	updateState('Flashlight ' + state);
	sendToCloud(command);
}

function turnLeft(num) {
	console.log('turn left: ' + num);
	
	updateState('Turning ' + num + ' degrees left');
	sendToCloud('TU_-' + num);
}

function forward() {
	console.log('go forward');
	
	toggleColor('forward', 'green');
	updateState('Moving forward');
	sendToCloud('FO_30');
}

function stop() {
	console.log('stop');
	
	toggleColor('stop', 'red');
	updateState('Stopping');
	sendToCloud('ST');
}

function back() {
	console.log('go back');

	toggleColor('back', 'green');
	updateState('Moving backward');
	sendToCloud('FO_-30');
}

function turnRight(num) {
	console.log('turn right: ' + num);
	
	updateState('Turning ' + num + ' degrees right');
	sendToCloud('TU_' + num);
}


// CSS function
function toggleColor(buttonId, color) {
	let classList = 	document.getElementById(buttonId).classList;
	classList.toggle(color);
	return classList.contains(color);
	
}

// Websockets
ws.onopen = function() {
	console.log('Connection to Cloud: ESTABLISHED');
}

ws.onclose = function() {
	console.log('Connection to Cloud: CLOSED');
}

ws.onerror = function(err) {
	console.log('Connection to Cloud: ERROR');
}


// Handling Cloud's messages
ws.onmessage = function(msg) {
	console.log('Message from Cloud: ' + msg.data);

	const states = {established: 'Resting', closed: '', error: ''};
	const data = JSON.parse(msg.data);
	if ('robot_conn' in data) {
		let classList = document.body.classList;
		for (const s in states) {
			const className = 'state-' + s;
			if (classList.contains(className)) classList.remove(className);
			if (data.robot_conn === s) {
				classList.add(className);
				updateState(states[s]);
			}
		}
	}  

	if('conf' in data && data.conf.includes('S_')) {
		let button;
		if (data.conf.includes('S_FO_30')) toggleColor('forward', 'green');
		if (data.conf.includes('S_FO_-30')) toggleColor('back', 'green');
/*		switch(data.conf) {
			case 'S_FO_30': 
				changeColor(document.getElementById('forward'), 'green');
				break;
			case 'S_ST':
				changeColor(document.getElementById('stop'), 'red');
				break;
			case 'S_FO_-30':
				changeColor(document.getElementById('back'), 'green');
				break;
			} */
		// update state to Resting
		updateState('Resting');
	}
}

	
