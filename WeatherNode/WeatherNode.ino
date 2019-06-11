/*
  This is derivitive of FSWebServer - Example WebServer with SPIFFS backend for esp8266.
  This program reads data from an SHT30 Temperature / Humidity sensor and displays vlaues on home.html.
  Starts as Access Point if first boot and lets you configure network access.
*/
/* If loaded without config file (or any files!) not savinng version in config file
    but then again, who cares?  no files, no webpages
    so need to program the load spiffs - can then assume legit config file exists
    need ot fix saving extra stuff in config file
*/

#include <ESP8266WiFi.h>
#include <WiFiClient.h>m
#include <ESP8266WebServer.h>
#include <WebSocketsServer.h>
#include <ESP8266mDNS.h>
#include <FS.h>
// Added to include JSON file support
#include <ArduinoJson.h>
// Added to include OTA support
#include <WiFiUdp.h>
#include <ArduinoOTA.h>
//Added to include OTA update via web browser
#include <ESP8266HTTPUpdateServer.h>
#include <WEMOS_SHT3X.h>
#define loopTime 30000

#define DEBUG                                            // Comment out this line to disable debug serial prints

#ifdef  DEBUG
#define DEBUG_PRINT(x)        Serial.print (x)
#define DEBUG_PRINTDEC(x)     Serial.print (x, DEC)
#define DEBUG_PRINTLN(x)      Serial.println (x)
#define DEBUG_PRINTF(x)       Serial.printf (x)
#else
#define DEBUG_PRINT(x)
#define DEBUG_PRINTDEC(x)
#define DEBUG_PRINTLN(x)
#define DEBUG_PRINTF(x)
#endif

#define ApPrefix              "LCLWN_"
#define maxConnectionAttempts 120    // 1 minute to attempt connection

// Set up the SHT30 sensor
SHT3X sht30(0x45);

char wnVersion[30] = "";
char ssid[30] = "default";
char password[30] = "LCLdefault";
char hostName[30] = "LCLdefault";
char temperatureChoice[10] = "C";

char apSSID[30] = "default";
char apPASS[30] = "default";
char rxBuffer[64];                                        // Recieved data serial buffer
String inSerial;                                          // String to hold rx'd data for string operations
String currentTemperature = "-100.0";
String currentHumidity = "-100.0";
String ioStatus = "";

unsigned char mac[6];
unsigned long currentStatusMillis = 0;
unsigned long previousStatusMillis = 0;
#define delayStatus 500

int temperatureReading = 0;

uint8_t blinkCount = 0;
uint8_t loopCounter = 0;

// *SIGNED* variables.  Positive and negative values are valid.
int8_t tempSlope = 1;
int8_t tempIntercept = 0;

uint8_t WiFiType = 0;
uint8_t started = 0;                                      // 0 = startup, 1 = loop

// Timers auxiliary variables
long now = millis();
long lastMeasure = 0;

// SETUP LEDS
const int LED1 = 14; //D5
const int LED2 = 12; //D6

ESP8266WebServer server(80);                              // start a htpp server named server on port 80
ESP8266HTTPUpdateServer httpUpdater;                      // start a http update server (for OTA updates)
WebSocketsServer webSocket = WebSocketsServer(81);        // start a web socket server named webSocket on port 81
File fsUploadFile;
//*****************************************************************************
// Setup                                                                      *
//*****************************************************************************
void setup(void) {

  pinMode(BUILTIN_LED, OUTPUT);
  pinMode(LED1, OUTPUT);
  pinMode(LED2, OUTPUT);

  Serial.begin(115200);
  Serial.printf("\n");
  Serial.setDebugOutput(false);                           // Enable WiFi serial debug output
  Serial.printf("Low Cost Labs Weather Node Starting.\n");
  SPIFFS.begin();
  digitalWrite(LED1, HIGH);
  digitalWrite(LED2, HIGH);
  delay(1000);
  digitalWrite(LED1, LOW);
  digitalWrite(LED2, LOW);
  {
    Dir dir = SPIFFS.openDir("/");
    while (dir.next()) {
      String fileName = dir.fileName();
      size_t fileSize = dir.fileSize();
#ifdef DEBUG
      Serial.printf("FS File: %s, size: %s\n", fileName.c_str(), formatBytes(fileSize).c_str());
#endif
    }
    DEBUG_PRINTF("\n");
  }
  // read config settings from JSON file
  if (!loadConfig()) {
    Serial.println("No config file found.  Using Defaults.");
  }
  delay(1000);
  //WIFI INIT
  Serial.println("Checking if first boot");
  if ((strcmp(ssid, "LCLdefault") == 0  )&&  (strcmp(password, "LCLdefault") == 0  )){
    WiFi.begin(ssid, password);
    WiFiType = 1;
    Serial.printf("First booot - default values detected.  Startign as Access Point.\n");
  } else {
    Serial.println("not first boot");
    DEBUG_PRINT("Connecting to "); DEBUG_PRINT(ssid);
    DEBUG_PRINT(" with Password "); DEBUG_PRINTLN(password);
    WiFi.begin(ssid, password);
    Serial.printf("Testing WiFi type\n");
    WiFiType = testWiFi();
  }
  if (WiFiType == 0) {
    DEBUG_PRINT("WiFi type is: "); DEBUG_PRINTLN(WiFiType);
    Serial.print("\n");
    Serial.printf("Starting Server\n");
    //setupServer();
    WiFi.softAPdisconnect(true);
    Serial.print("Connected! IP address: "); Serial.println(WiFi.localIP());
    // Start mDNS Service
    MDNS.begin(hostName);
    Serial.printf("Open http://%s.local/edit to see the file browser\n", hostName);
    Serial.printf("Open http://%s.local/list?dir= to list contents of file system\n", hostName);
    //Start HTTP OTA Service
    httpUpdater.setup(&server);  // For web based OTA Updates
    Serial.printf("HTTPUpdateServer ready! Open http://%s.local/update in your browser\n\n", hostName);
  } else {
    DEBUG_PRINT("WiFi type is: "); DEBUG_PRINTLN(WiFiType);
    Serial.println("Starting Access Point");
    delay(100);
    if (WiFiType == 1) {
      startAccessPoint();
    }
  }
  // Start Websocket Server
  DEBUG_PRINTLN("Connecting Websocket.");
  webSocket.begin();                                            // need to check failed websocket start and restart if required
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
  DEBUG_PRINTLN("HTTP server started\n");
  if (WiFiType == 0 ) {
    // Add OTA Support if connected to WiFi.
    // OTA update not availble when running as access point
    // ArduinoOTA.setPort(8266);                      // Port defaults to 8266
    Serial.println("Starting OTA Update server.");
    ArduinoOTA.setHostname(hostName);                     // Hostname defaults to esp8266-[ChipID]
    Serial.println("Hostname Set");
    // ArduinoOTA.setPassword((const char *)"123");   // No authentication by default
    ArduinoOTA.onStart([]() {
      Serial.println("Start");
    });
    ArduinoOTA.onEnd([]() {
      Serial.println("\nEnd");
    });
    ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) {
      Serial.printf("Progress: %u%%\r", (progress / (total / 100)));
    });
    ArduinoOTA.onError([](ota_error_t error) {
      Serial.printf("Error[%u]: ", error);
      if (error == OTA_AUTH_ERROR) Serial.println("Auth Failed");
      else if (error == OTA_BEGIN_ERROR) Serial.println("Begin Failed");
      else if (error == OTA_CONNECT_ERROR) Serial.println("Connect Failed");
      else if (error == OTA_RECEIVE_ERROR) Serial.println("Receive Failed");
      else if (error == OTA_END_ERROR) Serial.println("End Failed");
    });
    ArduinoOTA.begin();
    DEBUG_PRINTLN("Ready");
    DEBUG_PRINT("IP address: "); DEBUG_PRINTLN(WiFi.localIP());
  }
}
//*****************************************************************************
// Loop                                                                       *
//*****************************************************************************
void loop(void) {
  server.handleClient();
  webSocket.loop();
  ArduinoOTA.handle();
  server.handleClient();
  envStatus();
}
