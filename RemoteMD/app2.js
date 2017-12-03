
var express = require('express');
var app = express();

var routes = require('./routes/index.js');
var cloud = require('./routes/cloud.js');

app.use('/', routes);
app.use('/cloud', cloud);

var server = app.listen(80).on('error', console.log);