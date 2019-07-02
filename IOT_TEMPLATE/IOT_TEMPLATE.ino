/*
  This is derivitive of FSWebServer - Example WebServer with SPIFFS backend for esp8266.
  Serves webpages as requested, also processes Websocket requests
*/
// Include WiFi support
#include <ESP8266WiFi.h>
#include <WiFiClient.h>
// Include Webserver support
#include <ESP8266WebServer.h>
// Include Websocket support
#include <WebSocketsServer.h>
// Include mDNS support
#include <ESP8266mDNS.h>
#include <WiFiUdp.h>
// Include SPIFFs support
#include <FS.h>
// Include JSON file support
#include <ArduinoJson.h>

#define ApPrefix              "MYAP_"
#define maxConnectionAttempts 120    // 1 minute to attempt connection

char ssid[30] = "default";
char password[30] = "default";
char hostName[30] = "default";
char apSSID[30] = "default";
char apPASS[30] = "default";
char rxBuffer[64];                                        // Recieved data serial buffer
String inSerial;                                          // String to hold rx'd data for string operations

unsigned char mac[6];
unsigned long currentStatusMillis = 0;
unsigned long previousStatusMillis = 0;
#define delayStatus 500

uint8_t WiFiType = 0;
uint8_t started = 0;                                      // 0 = startup, 1 = loop

// Timers auxiliary variables
long now = millis();
long lastMeasure = 0;

ESP8266WebServer server(80);                              // start a htpp server named server on port 80
WebSocketsServer webSocket = WebSocketsServer(81);        // start a web socket server named webSocket on port 81
File fsUploadFile;
//*****************************************************************************
// Setup                                                                      *
//*****************************************************************************
void setup(void) {
  pinMode(2, OUTPUT);
  // GPIO 2 is also the Builtin LED on WEMOS D1 Mini and is active low.
  Serial.begin(115200);
  Serial.printf("\n");
  Serial.setDebugOutput(false);                           // Enable WiFi serial debug output
  Serial.printf("Ludzinc.blogspot.com IoT Template is starting.\n");
  SPIFFS.begin();
  {
    Dir dir = SPIFFS.openDir("/");
    while (dir.next()) {
      String fileName = dir.fileName();
      size_t fileSize = dir.fileSize();
      Serial.printf("FS File: %s, size: %s\n", fileName.c_str(), formatBytes(fileSize).c_str());
    }
    Serial.printf("\n");
  }
  // read config settings from JSON file
  if (!loadConfig()) {
    Serial.println("No config file found.  Using Defaults.");
  }
  //WIFI INIT
  Serial.println("Checking if first boot");
  Serial.print("SSID and PASSWORD ARE: "); Serial.print(ssid); Serial.print(":"); Serial.println(password);
  if ((strcmp(ssid, "default") == 0  ) &&  (strcmp(password, "default") == 0  )) {
    WiFi.begin(ssid, password);
    WiFiType = 1;
    Serial.printf("First booot - default values detected.  Starting as Access Point.\n");
  } else {
    Serial.println("not first boot");
    Serial.print("Connecting to "); Serial.print(ssid);
    Serial.print(" with Password "); Serial.println(password);
    WiFi.begin(ssid, password);
    Serial.printf("Testing WiFi type\n");
    WiFiType = testWiFi();
  }
  if (WiFiType == 0) {
    Serial.print("WiFi type is: "); Serial.println(WiFiType);
    Serial.print("\n");
    Serial.printf("Starting Server\n");
    //setupServer();
    WiFi.softAPdisconnect(true);
    Serial.print("Connected! IP address: "); Serial.println(WiFi.localIP());
    // Start mDNS Service
    MDNS.begin(hostName);
    Serial.printf("Open http://%s.local/edit to see the file browser\n", hostName);
    Serial.printf("Open http://%s.local/list?dir= to list contents of file system\n", hostName);
  } else {
    Serial.print("WiFi type is: "); Serial.println(WiFiType);
    Serial.println("Starting Access Point");
    delay(100);
    if (WiFiType == 1) {
      startAccessPoint();
    }
  }
  // Start Websocket Server
  Serial.println("Connecting Websocket.");
  webSocket.begin();
  webSocket.onEvent(webSocketEvent);
  server.begin();
  Serial.println("Websocket connected.");
  Serial.println("Starting mDNS service.");
  MDNS.addService("http", "tcp", 80);
  //Handle requests from client
  //list directory
  server.on("/list", HTTP_GET, handleFileList);
  //load editor
  server.on("/edit", HTTP_GET, []() {
    if (!handleFileRead("/edit.htm")) server.send(404, "text/plain", "FileNotFound");
  });
  //create file
  server.on("/edit", HTTP_PUT, handleFileCreate);
  //delete file
  server.on("/edit", HTTP_DELETE, handleFileDelete);
  //first callback is called after the request has ended with all parsed arguments
  //second callback handles file uploads at that location
  server.on("/edit", HTTP_POST, []() {
    server.send(200, "text/plain", "");
  }, handleFileUpload);
  //called when the url is not defined here
  //use it to load content from SPIFFS
  server.onNotFound([]() {
    if (!handleFileRead(server.uri()))
      server.send(404, "text/plain", "FileNotFound");
  });

  //get heap status, analog input value and all GPIO statuses in one json call
  server.on("/all", HTTP_GET, []() {
    String json = "{";
    json += "\"heap\":" + String(ESP.getFreeHeap());
    json += ", \"analog\":" + String(analogRead(A0));
    json += ", \"gpio\":" + String((uint32_t)(((GPI | GPO) & 0xFFFF) | ((GP16I & 0x01) << 16)));
    json += "}";
    server.send(200, "text/json", json);
    json = String();
  });
  server.begin();
  Serial.println("HTTP server started\n");
  digitalWrite(2, HIGH);
}//end setup()
//*****************************************************************************
// Loop                                                                       *
//*****************************************************************************
void loop(void) {
  server.handleClient();
  webSocket.loop();
  MDNS.update();
}//end loop()
