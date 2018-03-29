var express = require('express');
var http = require('http');
var WebSocket = require('ws');
var fs = require('fs');
var app = express();





// for user login
var mongoose = require('mongoose');
//var passport = require('passport');
//var flash = require('connect-flash');
// require('./config/passport')(passport); // pass passport for configuration

//var morgan = require('morgan');
var cookieParser = require('cookie-parser');
var bodyParser = require('body-parser');
var session = require('express-session');

//var configDB = require('./config/database.js');

//mongoose.connect(configDB.url); // connect to our database
mongoose.connect('mongodb://localhost:27017/remotemd'); // connect to our database

//app.use(morgan('dev')); // log every request to the console
app.use(cookieParser()); // read cookies (needed for auth)
app.use(bodyParser.urlencoded({ extended: true }));

//// required for passport
app.use(session({ secret: 'robotmdchangestheworld', resave: true, saveUninitialized: false })); // session secret
//app.use(passport.initialize());
//app.use(passport.session()); // persistent login sessions
//app.use(flash()); // use connect-flash for flash messages stored in session

//require('./routes/index.js')(app, passport); // load our routes and pass in our app and fully configured passport

var userSchema = new mongoose.Schema({
    username: { type: String, required: true, unique: true },
    password: { type: String, required: true },
    usertype: { type: String, required: true },
    ip: { type: String},
    doctors: { type: [String]}
});

var User = mongoose.model('User', userSchema);

// END user login 



var server = http.createServer(app);
const wss_browser = new WebSocket.Server({ server : server});

var routes = require('./routes/index');
app.use('/', routes);

// creating log file
var date = new Date();
var day = date.getDate(); 
var month = date.getMonth() + 1; 
var year = date.getFullYear(); 
var log_name = year  + '-' + month + '-' + day + '.log';
console.log(log_name);
var file = fs.createWriteStream(log_name, { flags: 'a'});

function writeToLog(data) {
  console.log(data);
  var date = new Date();
  var hour = date.getHours(); 
  var minutes = date.getMinutes(); 
  var seconds = date.getSeconds(); 
  var time = hour  + ':' + minutes + ':' + seconds + ': ';
  file.write(time + data + '\n');
} 

// only in Navigation page
wss_browser.on('connection', function connection(ws, req) {
  writeToLog('Connection to Browser: ESTABLISHED');

  const ws_robot = new WebSocket('ws://192.168.9.38:8080');
  //const ws_robot = new WebSocket('ws://localhost:8080');

  ws_robot.onopen = function () {
      writeToLog('Connection to Robot: ESTABLISHED');
      ws.send(JSON.stringify({ "robot_conn": "established" }));
  };
  
  ws_robot.onerror = function (error) {
      writeToLog('Connection to Robot: ERROR');
      ws.send(JSON.stringify({ "robot_conn": "error" }));
  };
  
  ws_robot.onclose = function () {
      writeToLog('Connection to Robot: CLOSED');
      ws.send(JSON.stringify({ "robot_conn": "closed" }));
  };


  // forwarding messages from robot to browser
  ws_robot.onmessage = function (msg) {
      var data = msg.data;
      writeToLog('Message from Robot: ' + data);
      // send message to browser
      ws.send(data);
  };

  // forwarding messages from browser to robot
  ws.on('message', function incoming(data) {
      writeToLog('Message from Browser: ' + data);
      // send message to robot
      ws_robot.send(data);
  }); 

  ws.on('error', function(error) {
    writeToLog('Connection to Browser: ERROR');
    // closing log file
    file.end();
  });

  ws.on('close', function() {
    writeToLog('Connection to Browser: CLOSED');
    // closing log file
    file.end();
  });
});


wss_browser.on('error', function(err) {
  writeToLog('Cloud Server: ERROR');
  // closing log file
  file.end();
});

// Login/Registration Requests
var urlencodedParser = bodyParser.urlencoded({ extended: false });

app.post('/register', urlencodedParser, function (req, res) {
    writeToLog(`Trying to register a ${req.body.usertype}: ${req.body.username} - ${req.body.password} - ${req.ip}`);

    var newUser = new User();
    newUser.username = req.body.username;
    newUser.password = req.body.password;
    newUser.usertype = req.body.usertype;
    newUser.ip = req.body.ip;
    newUser.save(function (err, savedUser) {
        if (err) {
            writeToLog(err);
            return res.redirect("/register.html?result=error");
        }
        writeToLog(`Registered a ${savedUser.usertype}: ${savedUser.username} - ${savedUser.password} - ${savedUser.ip}`);
        req.session.user = newUser;
        if (newUser.usertype === "doctor")
            return res.redirect("/doctor.html"); // Go to /doctor
        else
            return res.redirect("/patient.html"); // Go to /patient
    });
});

app.post('/updatemydoctors', urlencodedParser, function (req, res) {
    var user = req.session.user;
    if (!user)
        return res.redirect('/?result=unauthorized');
    if (req.param('submitbutton') === "back")
        return res.redirect('/');
    var docs = req.param('mydoctors');
    if (!docs)
        docs = [];

    writeToLog(`Trying to update ${user.username}'s doctors to: ${docs}`);

    User.findOneAndUpdate({ username: user.username }, { $set: { doctors: docs } }, { new: true }, function (err, savedUser) {
        if (err) {
            writeToLog(err);
            return res.redirect("/patient.html?result=error");
        }
        writeToLog(`Updated ${user.username}'s doctors to: ${savedUser.doctors}`);
        req.session.user = savedUser;
        return res.redirect("/patient.html?result=success");
    });
});

app.get('/getdocs', function (req, res) {
    var user = req.session.user;
    if (!user || user.usertype !== 'patient')
        return res.json({ error: 'Access denied' });
    User.find({ usertype: 'doctor'}, 'username', function (err, users) {
        if (err) {
            writeToLog(err);
            return res.json({ error: 'Something went wrong' });
        }
        var alldoctors = users.map(userObj => userObj.username);
        writeToLog(`Telling ${user.username} that their doctors are ${user.doctors}`);
        return res.json({ alldocs: alldoctors, mydocs: user.doctors });
    });
});

app.post('/login', urlencodedParser, function (req, res) {
    writeToLog(`User ${req.body.username} is trying to log in as a ${req.body.loginbutton}`);
    User.findOne({ username: req.body.username, password: req.body.password, usertype: req.body.loginbutton },
        function (err, user) {
            if (err) {
                writeToLog(err);
                return res.status(500).send();
            }
            if (!user) {
                return res.redirect("/?error=loginfailed");
            }
            req.session.user = user;    // Log user in
            if (user.usertype === "doctor")
                return res.redirect("/doctor.html"); // Go to /doctor
            else
                return res.redirect("/patient.html"); // Go to /patient
        });
});

app.get('/getpatients', function (req, res) {
    var user = req.session.user;
    if (!user || user.usertype !== 'doctor')
        return res.json({ error: 'Access denied' });
    User.find({ usertype: 'patient', doctors: user.username }, 'username', function (err, users) {
        if (err) {
            writeToLog(err);
            return res.json({error: 'Something went wrong'});
        }
        var patients = users.map(userObj => userObj.username);
        writeToLog(patients);
        return res.json({ mypatients: patients });
    });
});

app.post('/choosepatient', function (req, res) {
    var user = req.session.user;
    if (!user || user.usertype !== 'doctor')
        return res.redirect("/?error=unauthorized");
    if (req.param('submitbutton') === "back")
        return res.redirect('/');
    User.findOne({ username: req.body.mypatients, usertype: 'patient' },
        function (err, user) {
            if (err) {
                writeToLog(err);
                return res.json({ error: 'Something went wrong' });
            }
            req.session.ip = user.ip;
            return res.redirect(`/navigation.html?cloudIp=localhost&patient=${user.username}`);
        }
    );
});

server.listen(80).on('error', function() {
  writeToLog('Listening: ERROR');
  // closing log file
  file.end();
});


