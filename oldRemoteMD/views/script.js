function genericFetch(url, val) {
	fetch(url, {
		method: 'POST',
		headers: { "Content-type": "application/json" },
		body: JSON.stringify({value: val})
	}).then(response => {
			if (response.ok) { return response.json(); }
			throw new Error('Request failed!');
		}, networkError => console.log(networkError.message)
		).then(jsonResponse => { return jsonResponse; });
}



function flashlight() {
	console.log('flashlight');

	let buttonClass = document.getElementById('flash').classList;

	let url = '/cloud/flashlight/';
	url += (buttonClass.contains('lightOff') ? 'on' : 'off');
	
	
  fetch(url, { 
		method: 'POST', 
		headers: { "Content-type": "application/json"},
		body: JSON.stringify(value: val)}
	).then(response => {
   		    if (response.ok) { return response.json(); }
    	    throw new Error('Request failed!'); },
	    networkError => console.log(networkError.message)
	).then(jsonResponse => { 
		if (buttonClass.contains('lightOff')) {
			buttonClass.remove('lightOff');
			buttonClass.add('lightOn');
		} else {
				buttonClass.remove('lightOn');
				buttonClass.add('lightOff');
		}
			return jsonResponse; 
		}); 
};


function turnLeft(num) {
	console.log('turn left: ' + num);

	let url = '/cloud/turn/left/' + num;

  genericFetch(url, num);
};



function forward() {
	console.log('go forward');

	let buttonElement = document.getElementById('forward');
	
	let url = '/cloud/go/forward';
	
  fetch(url, { 
		method: 'POST', 
		headers: { "Content-type": "application/json"},
		body: JSON.stringify()}
	).then(response => {
   		 if (response.ok) { return response.json(); }
    	 throw new Error('Request failed!'); },
	  networkError => console.log(networkError.message)
		).then(jsonResponse => { 
				buttonElement.classList.remove('none');
				buttonElement.classList.add('green');
			
				window.setTimeout(() => {
					buttonElement.classList.remove('green');
					buttonElement.classList.add('none');
				}, 1000);
		
				return jsonResponse; 
			}); 
};

function back() {
	console.log('go back');

	let buttonElement = document.getElementById('forward');
	
	let url = '/cloud/go/back';
	
  fetch(url, { 
		method: 'POST', 
		headers: { "Content-type": "application/json"},
		body: JSON.stringify()}
	).then(response => {
   		 if (response.ok) { return response.json(); }
    	 throw new Error('Request failed!'); },
	  networkError => console.log(networkError.message)
		).then(jsonResponse => { 
				buttonElement.classList.remove('none');
				buttonElement.classList.add('green');
			
				window.setTimeout(() => {
					buttonElement.classList.remove('green');
					buttonElement.classList.add('none');
				}, 1000);
		
				return jsonResponse; 
			}); 
};

function turnRight(num) {
	console.log('turn right: ' + num);

	let url = '/cloud/turn/right/' + num;

  genericFetch(url, num);
};


// server-sent events
let update = new EventSource("/cloud/updates");

update.onmessage = function (event) {
	switch(data) {
	
};                


