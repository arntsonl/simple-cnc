var express = require('express');
var router = express.Router();

var cncDb = require('../cncdb');

var uuid = require('node-uuid');

/* GET home page. */
router.get('/', function(req, res) {
  res.render('index');
});

router.post('/login', function(req, res) {
  var login = req.body.login;
  var password = req.body.password;
  cncDb.login(login, password, function(userCnt) {
    if ( userCnt == 1 )
    {
      var sessionId = uuid.v4();
      cncDb.setSession(login, sessionId);
      res.cookie('session', sessionId, { maxAge: 900000 });
      res.redirect('/cpanel/');
    }
    else
      res.render('index');
  });
});

router.get('/cp', function(req, res) {

});

module.exports = router;
