
const spawn = require('child_process').spawn;
const child = spawn('../test');

process.stdin.pipe(child.stdin);

child.stdout.on('data', (data) => {
  console.log("Got: '" + data + "'");

});