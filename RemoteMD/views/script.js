const ws = new WebSocket('ws://35.197.216.190/');

ws.onopen = function() {
	console.log('connecting with cloud..');
}



function flashlight() {
	console.log('flashlight');

	let buttonClass = document.getElementById('flash').classList;

	let command = (buttonClass.contains('lightOff') ? 'FL_1' : 'FL_0');
	let msg = {"command" : command};

	ws.send(JSON.stringify(msg));
}


function turnLeft(num) {
	console.log('turn left: ' + num);

	let msg = {"command" : 'TU_-' + num};
	
	ws.send(JSON.stringify(msg));
};


function forward() {
	console.log('go forward');

	let msg = {"command" : 'FO_10'};
	
	ws.send(JSON.stringify(msg));
}


function back() {
	console.log('go back');

	let msg = {"command" : 'FO_-10'};
	
	ws.send(JSON.stringify(msg));
};


function turnRight(num) {
	console.log('turn right: ' + num);

	let msg = {"command" : 'TU_' + num};
	
	ws.send(JSON.stringify(msg));
};



function changeColorForFlash(buttonClass) {
	if (buttonClass.contains('lightOff')) {
		buttonClass.remove('lightOff');
		buttonClass.add('lightOn');
	} else {
		buttonClass.remove('lightOn');
		buttonClass.add('lightOff');
	}
};

function changeColorForMove(button) {
	button.classList.remove('none');
	button.classList.add('green');

	window.setTimeout(() => {
		button.classList.remove('green');
		button.classList.add('none');
	}, 1000);
};



ws.onmessage = function(msg) {
	console.log('got response from cloud: ' + msg.data);

	let buttonClass = document.getElementById('flash').classList;
	let json_msg = JSON.parse(msg.data);
	switch(json_msg.conf) {
		case 'S_FL_1': 
		case 'S_FL_0': {
			let buttonClass = document.getElementById('flash').classList;
			changeColorForFlash(buttonClass);
		} break;
		case 'S_FO_10': {
			let button = document.getElementById('forward');
			changeColorForMove(button);
		} break;
		case 'S_FO_-10': {
			let button = document.getElementById('back');
			changeColorForMove(button);
		} break;
	}
}

	
