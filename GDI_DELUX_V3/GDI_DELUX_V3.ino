/*
  This is derivitive of FSWebServer - Example WebServer with SPIFFS backend for esp8266
  Access the sample web page at http://hostName.local
  Edit the page by going to http://localName.local/edit
  List the contents of the file system with http://localName.local/list?dir=
*/
/*
   TO DO:

   RESET BOOTSTRAP BUTTONS - jQuery?
   Load css files (and javascript) locally if no access to CDS
   - test AP mode
   
   Update MODES for INPUTS
   0,1,2 for Door, Light or Monitor
   IF monitor, when door relay is triggered, check current monitor state
    - after delay time (optional) check if door is in alternate state.
    - if not in alternate state, retry (optional)
    - after failed max retries (user setting) BLINK all front panel leds
    Resume normal operation if button pressed / input fired / html reqst occurs.
    - Webpage can concatenate buttons( i.e. a 'both' button to trigger two doors at once)

    WERID BUG
      - if input mode != buttons mode, issues happen.
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
#include <Wire.h>

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

//#define status0 0x80
//#define status1 0x40
//#define status2 0x20
//#define status3 0x10

#define redLED      0x08
#define yellowLED   0x04
#define greenLED    0x02
#define blueLED     0x01

#define addressMCP23017       0x20    // I2C Address of the MCP23017 IO expander
#define addressTCN75A         0x48    // I2C address of the TCN75A temperature sensor
#define greenButtonLED        14
#define redButtonLED          16
#define ApPrefix              "LCLGDI_"
#define maxConnectionAttempts 120     // 1 minute to attempt connection

#define mcp23017PortA   0
#define mcp23017PortB   1
#define mcp23017Reset   2
#define setPins         1
#define clearPins       0

#define i2cSDA          12
#define i2cSCL          13

#define debounceCount   25

char gdiVersion[30] = "";
char ssid[30] = "";
char password[30] = "";
char hostName[30] = "";
char temperatureChoice[10] = "";

char apSSID[30] = "";
char apPASS[30] = "";
char rxBuffer[64];                                        // Recieved data serial buffer
String inSerial;                                          // String to hold rx'd data for string operations
String currentTemperature = "-100.0";
String ioStatus = "";
String htmlButtonNames[] = {"1", "2", "3", "4"};

unsigned char mac[6];
unsigned long currentStatusMillis = 0;
unsigned long previousStatusMillis = 0;
#define delayStatus 500
#define doorDelay   1000
#define lightDelay  5000

int temperatureReading = 0;
char ioPortA = 0xFF;
char ioPortB = 0xFF;
int pwmRedButton, pwmGreenButton = 0;
uint16_t panelButtonPWM[2];

typedef struct inputData {
  uint8_t state;
  uint8_t count;
  uint8_t current;
  uint8_t lock;
  uint8_t flag;
  uint32_t currentMillis; // change to uint32_t ??
  uint32_t previousMillis;
  uint8_t mode;
  uint8_t active;
  uint8_t set;
  uint8_t show;
  uint32_t onTime;
};

inputData buttons[4];
inputData inputs[4];
uint8_t currentInput = 0;
uint8_t lightON = 0;

uint8_t blinkCount = 0;
uint8_t loopCounter = 0;
//uint8_t flagRedLED = 0;
//uint8_t flagYellowLED = 0;
//uint8_t flagGreenLED = 0;
//uint8_t flagBlueLED = 0;
uint8_t redLEDFlag    = 0;
uint8_t yellowLEDFlag = 0;
uint8_t greenLEDFlag  = 0;
uint8_t blueLEDFlag   = 0;


// *SIGNED* variables.  Positive and negative values are valid.
int8_t tempSlope = 0;
int8_t tempIntercept = 0;

uint8_t WiFiType = 0;
uint8_t started = 0;                                      // 0 = startup, 1 = loop

ESP8266WebServer server(80);                              // start a htpp server named server on port 80
ESP8266HTTPUpdateServer httpUpdater;                      // start a http update server (for OTA updates)
WebSocketsServer webSocket = WebSocketsServer(81);        // start a web socket server named webSocket on port 81
File fsUploadFile;
//*****************************************************************************
// Setup                                                                      *
//*****************************************************************************
void setup(void) {

  pinMode(greenButtonLED, OUTPUT);
  pinMode(redButtonLED, OUTPUT);
  pinMode(mcp23017Reset, OUTPUT);
  pinMode(greenButtonLED, OUTPUT);
  pinMode(redButtonLED, OUTPUT);

  digitalWrite(mcp23017Reset, 0);
  delay(10);

  Serial.begin(115200);
  Serial.printf("\n");
  Serial.setDebugOutput(false);                           // Enable WiFi serial debug output
  Serial.printf("Low Cost Labs GDI Starting.\n");
  for (int i = 0; i < 4; i++) {                           // Initialise the input arrays (prevent output relays triggering at restart)
    initInputs(buttons[i]);
    initInputs(inputs[i]);
  }
  for (int i = 0; i < 2; i++) {
    panelButtonPWM[i] = 0;
  }
  initI2CModules();
  // Cycle front panel LEDs to show unit is booting
  writeIOModule(redLED, 0x00);
  Serial.println("RED LED ON.");
  delay(500);
  writeIOModule(yellowLED, 0x00);
  Serial.println("YELLOW LED ON.");
  delay(500);
  writeIOModule(greenLED, 0x00);
  Serial.println("GREEN LED ON.");
  delay(500);
  writeIOModule(blueLED, 0x00);
  Serial.println("BLUE LED ON.");
  delay(500);
  writeIOModule(0x0F, 0x00);
  SPIFFS.begin();
  delay(1000);
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
  loadConfig();
  DEBUG_PRINTF("Setting button PWM Values.\n");
  //  analogWrite(greenButtonLED, panelButtonPWM[0]);
  //  analogWrite(redButtonLED, panelButtonPWM[1]);
  writeIOModule(redLED, 0x00);                        // Turn of all but RED LED to show SPIFFS opened successfully
  delay(1000);
  //WIFI INIT
  DEBUG_PRINT("Connecting to "); DEBUG_PRINT(ssid);
  DEBUG_PRINT(" with Password "); DEBUG_PRINTLN(password);
  WiFi.begin(ssid, password);
  Serial.printf("Testing WiFi type\n");
  WiFiType = testWiFi();
  if (WiFiType == 0) {
    DEBUG_PRINT("WiFi type is: "); DEBUG_PRINTLN(WiFiType);
    Serial.print("\n");
    Serial.printf("Starting Server\n");
    blueLEDFlag = blueLED;                // BLUE on = WiFi connected
    delay(100);
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
    //    setPanelStatus(panelBLUE, setPins); // Blue LED = Wifi Connected
  } else {
    DEBUG_PRINT("WiFi type is: "); DEBUG_PRINTLN(WiFiType);
    Serial.println("Starting Access Point");
    delay(100);
    if (WiFiType == 1) {
      startAccessPoint();
    }
    //    flagGreenLED = 1;                                             // Green LED = Access Point
    // handle in blink routine - blink blue led when access point, check WiFiType flag in blink
  }
  // Start Websocket Server
  DEBUG_PRINTLN("Connecting Websocket.");
  webSocket.begin();                                            // need to check failed websocket start and restart if required
  webSocket.onEvent(webSocketEvent);
  server.begin();
  MDNS.addService("http", "tcp", 80);
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
  // Add OTA Support
  // ArduinoOTA.setPort(8266);                      // Port defaults to 8266
  ArduinoOTA.setHostname(hostName);                     // Hostname defaults to esp8266-[ChipID]
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
  //  flagRedLED = 1;
  redLEDFlag = redLED;             // Turn on Red led to show setup is completed
  analogWrite(greenButtonLED, panelButtonPWM[0]);
  analogWrite(redButtonLED, panelButtonPWM[1]);
}
//*****************************************************************************
// Loop                                                                       *
//*****************************************************************************
void loop(void) {
  server.handleClient();
  webSocket.loop();
  ArduinoOTA.handle();
  server.handleClient();
  timerFunctions();
  debounceInputs();
  setPanelLeds();
  blinkStatus();
}
