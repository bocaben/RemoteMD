//var cloudURL = 'http://192.168.43.xx' + ':8080';
var cloudURL = 'http://localhost' + ':8080';

var fetch = require('node-fetch');

const spawn = require('child_process').spawn;
global.child = spawn('../tcr/test.exe');

process.stdin.pipe(child.stdin);

global.child.stdout.on('data', (data) => {
    console.log("Got: '" + data + "'");
    genericFetch(cloudURL + '/cloud/conf/' + data);
});



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
