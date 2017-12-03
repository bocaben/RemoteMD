
var express = require('express');
var app = express();

var routes = require('./routes/index');
var cloud = require('./routes/cloud');

app.use('/', routes);
app.use('/cloud', cloud);

var server = app.listen(80).on('error', console.log);
console.log('Express server listening on port ' + server.address().port);