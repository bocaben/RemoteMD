const ws = new WebSocket('ws://localhost/');
//const ws = new WebSocket('ws://35.197.216.190/');

function sendToCloud(command) {
	ws.send(JSON.stringify({"command": command}));
}

// Navigation's buttons functions
function flashlight() {
	console.log('flashlight');

	let buttonClass = document.getElementById('flash').classList;
	let command = buttonClass.contains('on') ? 'FL_0' : 'FL_1';

	sendToCloud(command);
}

function turnLeft(num) {
	console.log('turn left: ' + num);
	sendToCloud('TU_-' + num);
}

function forward() {
	console.log('go forward');
	sendToCloud('FO_30');
}

function stop() {
	console.log('stop');
	sendToCloud('ST');
}

function back() {
	console.log('go back');
	sendToCloud('FO_-30');
}

function turnRight(num) {
	console.log('turn right: ' + num);
	sendToCloud('TU_' + num);
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


// AUX function
function toggleColor(buttonId, color) {
	let classList = document.getElementById(buttonId).classList;
	classList.toggle(color);
}

function updateState(str) {
	document.getElementsByTagName('p')[0].innerHTML = str;
}

function startsWith(str, prefix) {
	return str.indexOf(prefix) === 0;
}

function changeColorAndStateForCmd(msg) {
	const cmd = msg.substr(2);
	const commandToState = {'FL_1': 'Flashlight ON',
												'FL_0': 'Flashlight OFF',
												'TU_-90': 'Turning 90 degrees left',
												'TU_-45': 'Turning 45 degrees left',
												'TU_-20': 'Turning 20 degrees left',
												'FO_30': 'Moving forward',
												'ST': 'Stopping',
												'FO_-30': 'Moving backwards',
												'TU_20': 'Turning 20 degrees right',
												'TU_45': 'Turning 45 degrees right',
												'TU_90': 'Turning 90 degrees right'
											};
	if (cmd in commandToState) {
		let id;
		switch(cmd) {
			case 'FO_30': toggleColor('forward', 'on');	break;
			case 'ST': toggleColor('stop', 'on');	break;
			case 'FO_-30': toggleColor('back', 'on');	break;
			case 'FL_1':
			case 'FL_0': {
				if (!startsWith(msg, 'S_'))
					toggleColor('flash', 'on');
			}
		}

		if (startsWith(msg, 'C_')) updateState(commandToState[cmd]);
		if (startsWith(msg, 'F_')) updateState('FAILED ' + commandToState[cmd]);
		if (startsWith(msg, 'S_')) updateState('Resting');
	}
}

// Handling Cloud's messages
ws.onmessage = function(msg) {
	console.log('Message from Cloud: ' + msg.data);

	// handle Robot's state
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
	// handle confirm/success/failed messages
	if ('conf' in data)	changeColorAndStateForCmd(data.conf);
}

	
