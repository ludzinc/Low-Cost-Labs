// http://ejohn.org/blog/ecmascript-5-strict-mode-json-and-more/
"use strict";
// Optional. You will see this name in eg. 'ps' or 'top' command
process.title = 'Socket Server';
// Port where we'll run the websocket server
var webSocketsServerPort = 80;
// websocket and http servers
var webSocketServer = require('websocket').server;
var http = require('http');
// Global variables
// latest 100 messages
var history = [];
// list of currently connected clients (users)
var clients = [];
var SSIDs = ["HALLU", "SSID2", "SSID3"];
var RSSIs = ["RSSI1", "RSSI2", "RSSI3"];
var TYPEs = ["T1", "T2", "T3"];

var json = JSON.stringify({
  "echo": "default message"
});

var jsonMessage = "";
/**
 * Helper function for escaping input strings
 */
function htmlEntities(str) {
  return String(str).replace(/&/g, '&amp;').replace(/</g, '&lt;')
    .replace(/>/g, '&gt;').replace(/"/g, '&quot;');
}

function sendSSIDs() {
  console.log("SSID List requested");
  json = JSON.stringify({
    "echo": "listSSIDs",
    "SSIDName": ["SSID1", "SSID2"],
    "SSIDRSSI": [-101, -101],
    "SSIDType": ["TYPE 1", "TYPE 2"],
  });
}

function sendLocalName() {
  console.log('Local Name Requested.');
  json = JSON.stringify({
    "echo": "localName",
    "LocalName": "NewLocalName"
  });
}

function sendTempConfig() {
  console.log('Temperature Configuration Requested.');
  json = JSON.stringify({
    "echo": "configTemperature",
    "mode": "C",
    "slope": 1,
    "intercept": -5,
    "data": -95
  });
}

function sendTempData() {
  console.log('Temperature Data Requested.');
  json = JSON.stringify({
    "echo": "dataTemperature",
    "data": -123
  });
}

function sendButtonConfig() {
  console.log('Button Config Requested.');
  json = JSON.stringify({
    "echo": "configButtons",
    "buttons": [{
      "name": "Button 1",
      "mode": "door",
      "time": 500,
      "show": "true"
    }, {
      "name": "Button 2",
      "mode": "door",
      "time": 500,
      "show": "true"
    }, {
      "name": "Button 3",
      "mode": "door",
      "time": 500,
      "show": "false"
    }, {
      "name": "Light",
      "mode": "light",
      "time": 300000,
      "show": "true"
    }]
  });
}
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
// This callback function is called every time someone
// tries to connect to the WebSocket server
wsServer.on('request', function(request) {
  console.log((new Date()) + ' Connection from origin ' + request.origin + '.');
  // accept connection - you should check 'request.origin' to make sure that
  // client is connecting from your website
  // (http://en.wikipedia.org/wiki/Same_origin_policy)
  var connection = request.accept(null, request.origin);
  // we need to know client index to remove them on 'close' event
  var index = clients.push(connection) - 1;
  var userName = false;
  // var userColor = false;
  console.log((new Date().getHours()) + ':' + (new Date().getMinutes()) + ' Connection accepted.');
  // send back chat history
  if (history.length > 0) {
    connection.sendUTF(JSON.stringify({
      type: 'history',
      data: history
    }));
  }

  // user sent some message
  connection.on('message', function(message) {
    var messageRecieved = message.utf8Data;

    if (message.type === 'utf8') { // accept only text
      console.log('** Received Message : ' + message.utf8Data); //echo rx'd data
      // check what was requested
      json = JSON.stringify({
        "echo": "Invalid Message Recieved"
      });
      console.log("Default message (Error) ready");
      jsonMessage = JSON.parse(messageRecieved);
      console.log("Parseing JSON file.")
      console.log("type is :", jsonMessage.type)
      switch (jsonMessage.type) {
        case 'saveButtons':
          console.log("Type saveButtons recived");
          json = JSON.stringify({
            "echo": "Button Settings Saved"
          });
          break;
        case 'saveNetworkData':
          console.log("Type saveNetworkSettings recived");
          json = JSON.stringify({
            "echo": "Network Settings Saved"
          });
          break;
          case 'saveTemperature':
            console.log("Type saveTemperature recived");
            json = JSON.stringify({
              "echo": "Temperature Settings Saved"
            });
            break;
        case 'request':
          if (jsonMessage.data == 'localName') {
            sendLocalName();
          }
          if (jsonMessage.data == 'listSSIDs') {
            sendSSIDs();
          }
          if (jsonMessage.data == 'configButtons') {
            sendButtonConfig();
          }
          if (jsonMessage.data == 'configTemperature') {
            sendTempConfig();
          }
          if (jsonMessage.data == 'dataTemperature') {
            sendTempData();
          }
          break;
        default:
          console.log("Type unknown.")
      }
      console.log("Sending reply to request : ", messageRecieved);
      clients[0].sendUTF(json);
      console.log('Message sent is:' + json);
    }
  });
  // user disconnected
  connection.on('close', function(connection) {
    console.log((new Date().getHours()) + ':' + (new Date().getMinutes()) + " Peer disconnected.");
    // remove user from the list of connected clients
    clients.splice(index, 1);
  });
});
