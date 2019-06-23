// See
// https://gist.github.com/martinsik/2031681
//
/*jslint node: true */

"use strict";
// Optional. You will see this name in eg. 'ps' or 'top' command
process.title = 'Socket Server';
// Port where we'll run the websocket server
var webSocketsServerPort = 80;
// websocket and http servers
var webSocketServer = require('websocket').server;
var http = require('http');
var fs = require('fs');
// Global variables
var response = 'default';
// HTTP server
var server = http.createServer(function(request, response) {
  // Not important for us. We're writing WebSocket server, not HTTP server
});
server.listen(webSocketsServerPort, function() {
  console.log((new Date()) + " Server is listening on port " + webSocketsServerPort);
});
// WebSocket server
var wsServer = new webSocketServer({
  // WebSocket server is tied to a HTTP server. WebSocket request is just
  // an enhanced HTTP request. For more info http://tools.ietf.org/html/rfc6455#page-6
  httpServer: server
});
// This callback function is called every time someone tries to connect to the WebSocket server
wsServer.on('request', function(request) {
  console.log((new Date()) + ' Connection from origin ' + request.origin + '.');
  // accept connection - you should check 'request.origin' to make sure that
  // client is connecting from your website
  // (http://en.wikipedia.org/wiki/Same_origin_policy)
  var connection = request.accept(null, request.origin);
  // console.log('\x1b[1;32m%s%s\x1b[0m ',(new Date()), ' Connection accepted.');
  // (new Date().getHours()) + ':' + (new Date().getMinutes()) +
  var newDate = (new Date().getHours()) + ':' + (new Date().getMinutes());
  console.log('\x1b[1;32m%s%s\x1b[0m ', newDate, ' Connection accepted.');
  // user sent some message
  connection.on('message', function(message) {
    var messageRecieved = message.utf8Data;
    // var messageString = new String("\"echo\": \"NOT FOUND\"");
    if (message.type === 'utf8') { // accept only text
      console.log('\x1b[1;33m%s\x1b[0m%s ', '** Received Message : ', message.utf8Data);
      // check what was requested
      // console.log("Request recieved");
      var messageJSON = JSON.parse(messageRecieved);
      if (messageJSON.type == 'request') {
        // console.log("Request Recieved: " + messageJSON.data);
        var filename = messageJSON.data + '.json';
        console.log('\x1b[1;34m%s\x1b[0m%s', 'Attempting to open file: ', filename);
        fs.readFile(filename, 'utf8', function(err, contents) {
          if (err) {
            var errorMessage = (filename + ' not found.');
            console.log('\x1b[1;31m%s\x1b[0m%s ', errorMessage, ' Sending NOT FOUND response.');
            response = JSON.stringify({
              "echo" : "FILE NOT FOUND"
            });
          } else {
            response = contents;
          }
          connection.sendUTF(response);
          console.log('\x1b[1;96m%s\x1b[0m%s', 'Message sent is: \n', response);
        })
      } else if (messageJSON.type.startsWith('save')) {
        console.log('\x1b[1;92m%s\x1b[0m', 'Save Data request recieved: ', messageJSON.type);
        console.log("Recieved Type is: ", messageJSON.type);
        response = JSON.stringify({
          "echo" : messageJSON.type
        });
        connection.sendUTF(response);
        console.log('\x1b[1;96m%s\x1b[0m%s', 'Message sent is: \n', response);
      } else {
        console.log('\x1b[1;91m%s\x1b[0m', 'Invalid Message Recieved.');
      }
    }
  });
  // user disconnected
  connection.on('close', function(connection) {
    newDate = (new Date().getHours()) + ':' + (new Date().getMinutes());
    console.log('\x1b[1;32m%s%s\x1b[0m ', newDate, ' Peer disconnected.');
  });
});
