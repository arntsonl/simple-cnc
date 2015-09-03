var express = require('express');
var router = express.Router();

var cncDb = require('../cncdb');

function parseCookies (request) {
    var list = {},
        rc = request.headers.cookie;

    rc && rc.split(';').forEach(function( cookie ) {
        var parts = cookie.split('=');
        list[parts.shift().trim()] = decodeURI(parts.join('='));
    });

    return list;
}


/* GET users listing. */
router.get('/', function(req, res) {
  var cookies = parseCookies(req);
  cncDb.getSession(cookies.session, function(userCnt){
    if ( userCnt == 1 )
      res.render('cpanel');
    else
      res.render('index');
  });
});

router.get('/commandmanager', function(req, res){
  var cookies = parseCookies(req);
  cncDb.getSession(cookies.session, function(userCnt){
    if ( userCnt != 1 ) return res.render('index');

    cncDb.getCommands(function(commands) {
      res.render('commandmanager', {commands: commands});
    });
  });
});

router.post('/newcommand', function(req, res){
  var cookies = parseCookies(req);
  cncDb.getSession(cookies.session, function(userCnt){
    if ( userCnt != 1 || (req.body.name && req.body.name == "")) return res.render('index');

    cncDb.addCommand(req.body.name, req.body.type, req.body.params, function() {
      cncDb.getCommands(function(commands) {
        res.render('commandmanager', {commands: commands});
      });
    });
  });
});

router.get('/botmanager', function(req, res){
  var cookies = parseCookies(req);
  cncDb.getSession(cookies.session, function(userCnt){
    if ( userCnt != 1 ) return res.render('index');

    cncDb.getBots(function(bots) {
      res.render('botmanager', {bots: bots});
    });
  });
});

router.get('/bot', function(req, res) {
  var cookies = parseCookies(req);
  cncDb.getSession(cookies.session, function(userCnt){
    if ( userCnt != 1 ) return res.render('index');

    var botname = req.query.name;
    cncDb.getBot(botname, function(bot) {
      cncDb.getCommands(function(commands) {
        res.render('bot', {bot: bot, commands: commands});
      });
    });
  });  
});

router.post('/updateBot', function(req, res) {
  var cookies = parseCookies(req);
  cncDb.getSession(cookies.session, function(userCnt){
    if ( userCnt != 1 ) return res.render('index');

    cncDb.updateBot(req.body.name, req.body.commandname, function() {
      cncDb.getBots(function(bots) {
        res.render('botmanager', {bots: bots});
      });
    });
  });
});

/* GET users listing. */
router.get('/logout', function(req, res) {
  res.clearCookie('session');
  res.redirect('/');
});

module.exports = router;
