
function flashlight() {
	console.log('flashlight');

	let buttonClass = document.getElementById('flash').classList;

	let url = '/cloud/flashlight/';
	url += (buttonClass == 'lightOff' ? 'on' : 'off');
	
	
  fetch(url, { 
		method: 'POST', 
		headers: { "Content-type": "application/json"},
		body: JSON.stringify()}
	).then(response => {
   		 if (response.ok) { return response.json(); }
    	 throw new Error('Request failed!'); },
	  networkError => console.log(networkError.message)
		).then(jsonResponse => { 
				if (buttonClass == 'lightOff') {
					buttonClass.remove('lightOff');
					buttonClass.add('lightOn');
				} else {
						buttonClass.remove('lightOn');
						buttonClass.add('lightOff');
				}
				return jsonResponse; 
			}); 
};


function turnLeft() {
	console.log('turnLeft');

	let url = '/cloud/move/left';

  fetch(url, { 
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
};


function stop() {
	console.log('stop');

	let buttonElement = document.getElementById('stop');
	
	let url = '/cloud/stop';
	
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
				buttonElement.classList.add('red');
			
				window.setTimeout(() => {
					buttonElement.classList.remove('red');
					buttonElement.classList.add('none');
				}, 1000);
				return jsonResponse; 
			}); 
};


function go() {
	console.log('go');

	let buttonElement = document.getElementById('go');
	
	let url = '/cloud/move';
	
	
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



function turnRight() {
	console.log('turnRight');

	let url = '/cloud/move/right';

  fetch(url, { 
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
};



