const ws = new WebSocket('ws://localhost/');
//const ws = new WebSocket('ws://35.197.216.190/');


// Navigation's buttons functions

function flashlight() {
	console.log('flashlight');

	let buttonClass = document.getElementById('flash').classList;

	// update state
	let p = document.getElementsByTagName('p')[0];
	let command = 'FL_1';
	let state = 'ON';

	if (buttonClass.contains('yellow')) {
		command = 'FL_0';
		state = 'OFF';
	}
	// update state
	changeColor(document.getElementById('flash'), 'yellow');
	p.innerHTML = 'Flashlight ' + state;

	ws.send(JSON.stringify({"command" : command}));
}

function turnLeft(num) {
	console.log('turn left: ' + num);
	// update state
	let p = document.getElementsByTagName('p')[0];
	p.innerHTML = 'Turning ' + num + ' degrees left';

	let msg = {"command" : 'TU_-' + num};
	ws.send(JSON.stringify(msg));
};

function forward() {
	console.log('go forward');
	// update state
	let p = document.getElementsByTagName('p')[0];
	changeColor(document.getElementById('forward'), 'green');
	p.innerHTML = 'Moving forward';

	let msg = {"command" : 'FO_30'};
	ws.send(JSON.stringify(msg));
}

function stop() {
	console.log('stop');
	// update state
	let p = document.getElementsByTagName('p')[0];
	changeColor(document.getElementById('stop'), 'red');
	p.innerHTML = 'Stopping';

	let msg = {"command" : 'ST'};
	ws.send(JSON.stringify(msg));
}

function back() {
	console.log('go back');
	// update state
	let p = document.getElementsByTagName('p')[0];
	changeColor(document.getElementById('back'), 'green');
	p.innerHTML = 'Moving backward';

	let msg = {"command" : 'FO_-30'};
	ws.send(JSON.stringify(msg));
};

function turnRight(num) {
	console.log('turn right: ' + num);
	// update state
	let p = document.getElementsByTagName('p')[0];
	p.innerHTML = 'Turning ' + num + ' degrees right';

	let msg = {"command" : 'TU_' + num};
	ws.send(JSON.stringify(msg));
};


// Disable/Enable navigation
function enableNavigation() {
	let buttons = document.getElementsByClassName('ctrl');
	let i;
	for (i = 0; i < buttons.length; i ++) {
		buttons[i].style.pointerEvents =  'auto';
	}
	// update state to Resting
	let p = document.getElementsByTagName('p')[0];
	p.innerHTML = 'Resting';
}

function disableNavigation() {
	let buttons = document.getElementsByClassName('ctrl');
	let i;
	for (i = 0; i < buttons.length; i ++) {
		buttons[i].style.pointerEvents =  'none';
		buttons[i].style.backgroundColor = 'white';
	}
	// update state to Resting
	let p = document.getElementsByTagName('p')[0];
	p.innerHTML = '';
}

// CSS function
function changeColor(button, color) {
	let buttonClass = button.classList;
	if (buttonClass.contains('none')) {
		buttonClass.remove('none');
		buttonClass.add(color);
	} else {
		buttonClass.remove(color);
		buttonClass.add('none');
	}
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

	let data = JSON.parse(msg.data);
	if ('robot_conn' in data) {
		let img = document.getElementById('connection');
		switch(data.robot_conn) {
			case 'established': 
				img.src = "icons/on.png"; 
				enableNavigation();
				break;
				
			case 'closed': 
				disableNavigation(); 
				img.src = "icons/off.png";
				return;

			case 'error': 
			  disableNavigation();
				img.src = "icons/error.png";
				return;
		}
	} else  if('conf' in data && data.conf.includes('S_')) {
						let button;
						if (data.conf.includes('S_FO_30')) changeColor(document.getElementById('forward'), 'green');
						if (data.conf.includes('S_FO_-30')) changeColor(document.getElementById('back'), 'green');
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
						let p = document.getElementsByTagName('p')[0];
						p.innerHTML = 'Resting';
					}
}

	
