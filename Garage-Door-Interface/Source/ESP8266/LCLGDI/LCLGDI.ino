// TO DO
// Change SSID processing to 32 bytes fixed length fields
// - Will need to pass length of the SSID from the inpout box too
#include <Arduino.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>
#include <ESP8266HTTPUpdateServer.h>
#include <WebSocketsServer.h>
#include <ArduinoOTA.h>
#include <ESP8266WiFi.h>
#include <ESP8266mDNS.h>
#include <FS.h>
#include <stdio.h>
#include <string.h>
#include <Hash.h>
#include "EEPROMAnything.h"

#define GREENLED 0
#define BLUELED  2
//***************************************************************************
// Global data section.                                                     *
//***************************************************************************
WebSocketsServer webSocket = WebSocketsServer(81);  // start a web socket server named webSocket on port 81
ESP8266WebServer httpServer(80);                    // start a htpp server named httpServer on port 80
ESP8266HTTPUpdateServer httpUpdater;
// Access Point Configuration
const char* apSSID = "LCLGDI";
const char* apPASS = "LetMeIn1";
// Station Mode Configuration
char LCLversion[10] = "LCLGDI_01";
char ssid[34] = "LUDZINC";
char password[65] = "Renegade!";
char localName[33] = "LCLGDI";
char tempSetting[2] = "C";
// EEPROM Structure
struct ESP_EEPROM {
  char s_LCLversion[10];
  char s_ssid[34];
  char s_password[65];
  char s_localName[33];
  char s_tempSetting[2];
} EEPROMSettings;
ESP_EEPROM EEPROMValidate;

char   rxBuffer[64];

bool wifiAP = false;

const char* ioStatusDefault   = "I:D10 D20";

#define DEBUG                 // Comment out this line to disable debug printing

#ifdef DEBUG
#define DEBUG_PRINT(x)      Serial.print (x)
#define DEBUG_PRINTDEC(x)   Serial.print (x, DEC)  // print as ASCII encoded decimal
#define DEBUG_PRINTLN(x)    Serial.println (x)
#else
#define DEBUG_PRINT(x)
#define DEBUG_PRINTDEC(x)
#define DEBUG_PRINTLN(x)
#endif

String ioStatus;
String currentTemperature = "-ttt.t";

String tableLines[10];

//***************************************************************************
// setup()                                                                  *
//***************************************************************************
void setup() {
  pinMode(GREENLED, OUTPUT);                                   // active high
  pinMode(BLUELED,  OUTPUT);                                   // active high
  digitalWrite(GREENLED, LOW);                                //Turn on both LEDs at boot
  digitalWrite(BLUELED, LOW);
  delay(1000);
  digitalWrite(GREENLED, HIGH);                                 //Turn on both LEDs at boot
  digitalWrite(BLUELED, HIGH);
  // Serial.begin(115200);
  Serial.begin(9600);
  Serial.println();
  Serial.println("LCL GDI ESP8266 Starting");
  EEPROM.begin(512);                                     // reserve 512 bytes of EEPROM - make lower?
  delay(100);
  //Set default GPIO States
  // TO DO - AS ABOVE
  //Read settings from EEPROM
  Serial.println("Reading EEPROM Structure: ");
  readEEPROMSettings();
  DEBUG_PRINTLN("Attempting to Connect to local WiFi network");
  wifiAP = WifiConnect();
  DEBUG_PRINT("wifiAP is: ");
  DEBUG_PRINTLN(wifiAP);
  if (wifiAP == 0) {
    MDNSConnect();      // If successful, setup MDNS.
  }
  HTTPUpdateConnect();
  WebSocketConnect();
  setupSPIFFS();
  servePages();
  ioStatus = ioStatusDefault;
}

void loop() {

  if (wifiAP == 0) {
    if (WiFi.status() != WL_CONNECTED) {
      WifiConnect();
      WebSocketConnect();
      MDNSConnect();
    }
  }
  webSocket.loop();
  httpServer.handleClient();
  //  getConfig();
}

//***************************************************************************
// Websockets Events                                                        *
//***************************************************************************
void webSocketEvent(uint8_t num, WStype_t type, uint8_t * payload, size_t length) {

  switch (type) {
    case WStype_DISCONNECTED:   // Add code here to execute if disconnected
      DEBUG_PRINTLN("Websocket disconnected");
      break;
    case WStype_CONNECTED:      // Add code here to execute when connected
      {
        getConfig();
        IPAddress ip = webSocket.remoteIP(num);
        // Get ip address of connection
        DEBUG_PRINT("Websocket IP Address is: ");
        DEBUG_PRINTLN(ip);
      }
      break;

    case WStype_TEXT:           // Execute code here to match text etc
      {
        //        getConfig();
        uint8_t responseLength = currentTemperature.length();
        String text = String((char *) &payload[0]);
        String textMessage, tempString1, tempString2 = "";
        Serial.print("WStype_TEXT recieved was: ");
        Serial.println(text);
        if (text.startsWith("request_")) {
          //DEBUG_PRINTLN(text[8]);
          digitalWrite(BLUELED, LOW);
          delay(500);
          digitalWrite(BLUELED, HIGH);
          if (text.startsWith("Temperature", 8)) {
            webSocket.sendTXT(num, currentTemperature.c_str(), currentTemperature.length());  // Send temperature data
            DEBUG_PRINT("Sending WStype_TEXT:");
            DEBUG_PRINTLN(currentTemperature.c_str());
          }
          if (text.startsWith("Status", 8)) {
            tempString1 = localName;
            tempString2 = tempSetting;
            textMessage = "Status#" + tempString1 + "#" + tempString2;
            DEBUG_PRINT("Preparing to send: ");
            DEBUG_PRINTLN(textMessage);
            webSocket.sendTXT(num, textMessage.c_str(), textMessage.length());  // Send update data
            DEBUG_PRINT("Sending WStype_TEXT:");
            DEBUG_PRINTLN(localName);
          }
          if (text.startsWith("listSSIDs", 8)) {
            textMessage = "";
            listSSIDs();
            DEBUG_PRINTLN("Preparing to send list of SSIDS: ");
            for (int i = 0; i < 10; i++) {
              if (tableLines[i] != "") {
                textMessage = tableLines[i];
                webSocket.sendTXT(num, textMessage.c_str(), textMessage.length());  // Send update data
              } else {
                if (i == 0) {
                  textMessage = "No SSIDs Found.";
                } else {
                  textMessage = "No more SSIDs Found.";
                  webSocket.sendTXT(num, textMessage.c_str(), textMessage.length());
                  i = 10;
                }
              }
            }
          }
        }
        if (text.startsWith("update_")) {
          String successMessage = "Settings Saved.";
          String failMessage = "Update Failed.";
          digitalWrite(BLUELED, LOW);
          delay(500);
          digitalWrite(BLUELED, HIGH);
          //
          // need to check if list or settings data
          //
          if (checkSaveOptions(text) == true) {
            webSocket.sendTXT(num, successMessage.c_str(), successMessage.length());  // Send update data
            Serial.println("Sending WStype_TEXT: Settings Saved.");
          } else {
            webSocket.sendTXT(num, failMessage.c_str(), failMessage.length());  // Send update data
            Serial.println("Sending WStype_TEXT: Settingns Failed.");
          }
        }

      }
      break;
    case WStype_BIN:
      DEBUG_PRINT("WStype_BIN recieved.");
      hexdump(payload, length);                 // echo data back to browser
      webSocket.sendBIN(num, payload, length);
      break;
  }
}
//***************************************************************************
// WiFi Setup                                                               *
//***************************************************************************
uint8_t WifiConnect() {

  bool APrequired = 0;

  WiFi.mode(WIFI_STA);
  Serial.print("Connecting to ");
  Serial.println(ssid);
  DEBUG_PRINT("With password ");
  DEBUG_PRINTLN(password);

  WiFi.begin(ssid, password);
  // check if connected to wifi.  If fail, set up Access Point
  APrequired = testWiFi();
  //  while (WiFi.status() != WL_CONNECTED) {
  //    DEBUG_PRINT(".");
  //    delay(500);
  //  }
  if (!APrequired) {
    Serial.print("Connected to ");
    Serial.print(ssid);
    Serial.print(" with IP address: ");
    Serial.println(WiFi.localIP());
    return 0;
  }
  else {
    DEBUG_PRINTLN("Set up AP - TO DO!");
    setupAP();
    // when an access point - reboot each hour and check for wifi access (if not ouut of the box defaults)
    return 1;
  }
}
//***************************************************************************
// Connect Websocket.                                                       *
//***************************************************************************
void WebSocketConnect() {
  //  DEBUG_PRINTLN("Connecting Websocket.");
  webSocket.begin();
  webSocket.onEvent(webSocketEvent);
}
//***************************************************************************
// MDNS Setup                                                               *
//***************************************************************************
void MDNSConnect() {
  //  if (!MDNS.begin(localName)) {
  if (!MDNS.begin(localName, WiFi.localIP())) {
    DEBUG_PRINTLN("Error setting up MDNS responder!");
    while (1) {
      delay(1000);
    }
  }
  DEBUG_PRINTLN("mDNS responder started");
  DEBUG_PRINT("Local name is:");
  DEBUG_PRINTLN(localName);
  MDNS.addService("ws", "tcp", 81);               // Any reason why websocket uses different port?
  MDNS.addService("http", "tcp", 80);
  digitalWrite(GREENLED, LOW);
}
//***************************************************************************
// HTTP Updater (for OTA)                                                   *
//***************************************************************************
void HTTPUpdateConnect() {
  httpUpdater.setup(&httpServer);
  httpServer.begin();
  if (!wifiAP) {
    Serial.println("HTTPUpdateServer ready!");
    Serial.print("Open http://");
    Serial.print(localName);
    Serial.println(".local in your browser\n");
  }
}
//***************************************************************************
// setupSPIFFS                                                              *
//***************************************************************************
void setupSPIFFS() {
  FSInfo fs_info;  // Info about SPIFFS
  Dir dir;         // Directory struct for SPIFFS
  File f;          // Filehandle
  String filename; // Name of file found in SPIFFS

  SPIFFS.begin(); // Enable file system

  // Show some info about the SPIFFS
  uint16_t cnt = 0;
  SPIFFS.info(fs_info);
#ifdef DEBUG
  dbg_printf("SPIFFS Files\nName                           -      Size");
  dir = SPIFFS.openDir("/"); // Show files in FS

  while (dir.next()) // All files
  {
    f = dir.openFile("r");
    filename = dir.fileName();
    dbg_printf("%-30s - %9s", // Show name and size
               filename.c_str(), formatBytes(f.size()).c_str());
    cnt++;
  }

  dbg_printf("%d Files, %s of %s Used", cnt,
             formatBytes(fs_info.usedBytes).c_str(),
             formatBytes(fs_info.totalBytes).c_str());
  dbg_printf("%s Free\n",
             formatBytes(fs_info.totalBytes - fs_info.usedBytes).c_str());
#endif
}
//***************************************************************************
// dgb_print                                                                 *
//***************************************************************************
void dbg_printf(const char *format, ...) {
  static char sbuf[1400];                         // For debug lines
  va_list varArgs;                                // For variable number of params
  va_start(varArgs, format);                      // Prepare parameters
  vsnprintf(sbuf, sizeof(sbuf), format, varArgs); // Format the message
  va_end(varArgs);                                // End of using parameters
  DEBUG_PRINTLN(sbuf);
}
//***************************************************************************
// formatBytes                                                              *
//***************************************************************************
String formatBytes(size_t bytes) {
  if (bytes < 1024) {
    return String(bytes) + " B";
  } else if (bytes < (1024 * 1024)) {
    return String(bytes / 1024.0) + " KB";
  } else if (bytes < (1024 * 1024 * 1024)) {
    return String(bytes / 1024.0 / 1024.0) + " MB";
  } else {
    return String(bytes / 1024.0 / 1024.0 / 1024.0) + " GB";
  }
}
//***************************************************************************
// servePages(): Serve requested page if exists, or return 404              *
//***************************************************************************
void servePages() {
  // Define URIs to be processed
  // list directory
  httpServer.on("/list", HTTP_GET, handleFileList);
  // Called when the url is not defined here
  // Use it to load content from SPIFFS
  httpServer.on("/", HTTP_GET, []() {
    if (!handleFileRead("/main.html")) httpServer.send(404, "text/plain", "FileNotFound");
  });
  //get heap status, analog input value and all GPIO statuses in one json call
  httpServer.on("/all", HTTP_GET, []() {
    String json = "{";
    json += "\"heap\":" + String(ESP.getFreeHeap());
    json += ", \"analog\":" + String(analogRead(A0));
    json += ", \"gpio\":" + String((uint32_t)(((GPI | GPO) & 0xFFFF) | ((GP16I & 0x01) << 16)));
    json += "}";
    httpServer.send(200, "text/json", json);
    json = String();
  });
  httpServer.onNotFound([]() {
    String requestedUri = httpServer.uri();
    DEBUG_PRINT("Requested URI is: ");
    DEBUG_PRINTLN(requestedUri);
    if (requestedUri.startsWith("/gpio_0")) {   // see if a gpio state has been called
      DEBUG_PRINTLN("process gpio");
      ioStatus.replace("D10", "D1A");           // Pint door state to serial for micro to action
      httpServer.send(204);                     // Request Processed, no content to return.
      Serial.println(ioStatus);
      digitalWrite(GREENLED, !digitalRead(GREENLED));  // blink LED 0
      delay(500);
      digitalWrite(GREENLED, !digitalRead(GREENLED));
      ioStatus = ioStatusDefault;
    } else if (requestedUri.startsWith("/gpio_2")) {
      DEBUG_PRINTLN("process gpio");
      ioStatus.replace("D20", "D2A");
      httpServer.send(204);                     // Request Processed, no content to return.
      Serial.println(ioStatus);
      digitalWrite(BLUELED, !digitalRead(BLUELED));  // blink LED 0
      delay(500);
      digitalWrite(BLUELED, !digitalRead(BLUELED));
      ioStatus = ioStatusDefault;
    }
    if (requestedUri.startsWith("/saveOptions")) {  // Look for SaveOptions request
      DEBUG_PRINTLN("process Save Options");
      bool saveSuccess = false;

      saveSuccess = checkSaveOptions(requestedUri);

      // trigger a websocket to mark success or failure of data here???  no I really want an event! Dammit!!
      // nope - when pressinng save, leave connection open until I recieve success message (or not).
      if (saveSuccess == true) {
        httpServer.send(204);                     // Request Processed, no content to return.
        // flag for websocket success message here
      } else {
        // flag for websocket fail message here
        httpServer.send(204);
      }
    }
    if (requestedUri.startsWith("/listSSID")) {  // Look for list available ssids in area request
      DEBUG_PRINTLN("process listSSID");
      listSSIDs();
      if (!handleFileRead("/listSSID.html")) httpServer.send(404, "text/plain", "FileNotFound");
    }
    else {
      if (!handleFileRead(httpServer.uri()))    // see if file exists in spiffs that matches request, otherwise return 404
        // i.e. /favicon.ico will return favicon.ico file (if present) or /main.html will return the main page.
        httpServer.send(404, "text/plain", "FileNotFound");
    }
  });
}
//***************************************************************************
// getContentType(): return file type                                       *
//***************************************************************************
String getContentType(String filename) {
  if (httpServer.hasArg("download")) return "application/octet-stream";
  else if (filename.endsWith(".htm")) return "text/html";
  else if (filename.endsWith(".html")) return "text/html";
  else if (filename.endsWith(".css")) return "text/css";
  else if (filename.endsWith(".js")) return "application/javascript";
  else if (filename.endsWith(".png")) return "image/png";
  else if (filename.endsWith(".gif")) return "image/gif";
  else if (filename.endsWith(".jpg")) return "image/jpeg";
  else if (filename.endsWith(".ico")) return "image/x-icon";
  else if (filename.endsWith(".xml")) return "text/xml";
  else if (filename.endsWith(".pdf")) return "application/x-pdf";
  else if (filename.endsWith(".zip")) return "application/x-zip";
  else if (filename.endsWith(".gz")) return "application/x-gzip";
  return "text/plain";
}
//***************************************************************************
// handleFileRead():  If requested file exists, serve.                      *
//***************************************************************************
bool handleFileRead(String path) {
  Serial.println("handleFileRead: " + path);
  if (path.endsWith("/")) path += "index.htm";
  String contentType = getContentType(path);
  String pathWithGz = path + ".gz";
  if (SPIFFS.exists(pathWithGz) || SPIFFS.exists(path)) {
    if (SPIFFS.exists(pathWithGz))
      path += ".gz";
    File file = SPIFFS.open(path, "r");
    size_t sent = httpServer.streamFile(file, contentType);
    file.close();
    return true;
  }
  return false;
}
//***************************************************************************
// handleFileList():  I DONT KNOW HOW THIS WORKS YET                        *
//***************************************************************************
void handleFileList() {
  if (!httpServer.hasArg("dir")) {
    httpServer.send(500, "text/plain", "BAD ARGS");
    return;
  }

  String path = httpServer.arg("dir");
  DEBUG_PRINTLN("handleFileList: " + path);
  Dir dir = SPIFFS.openDir(path);
  path = String();

  String output = "[";
  while (dir.next()) {
    File entry = dir.openFile("r");
    if (output != "[") output += ',';
    bool isDir = false;
    output += "{\"type\":\"";
    output += (isDir) ? "dir" : "file";
    output += "\",\"name\":\"";
    output += String(entry.name()).substring(1);
    output += "\"}";
    entry.close();
  }

  output += "]";
  httpServer.send(200, "text/json", output);
}
//***************************************************************************
// testWiFi(): Attempt to connect to wifi, on fail set flag to config as AP *
//***************************************************************************
int testWiFi() {
  int c = 0;
  Serial.println("Waiting for Wifi to connect.");
  while ( c < 120 ) {
    //  while ( c < 4 ) {
    Serial.print(".");
    digitalWrite(GREENLED, LOW);
    delay(500);
    digitalWrite(GREENLED, HIGH);
    delay(500);
    c++;
    if (WiFi.status() == WL_CONNECTED) {
      Serial.println("");
      return 0;
    }
  }
  Serial.println("");
  Serial.print("WiFi Status: ");
  Serial.println(WiFi.status());
  Serial.println("Connect timed out, opening AP");
  return 1;
}//end testWiFi()
//***************************************************************************
// checkSaveOptions(): Validate returned data from settings page            *
// to do - check for crap data in local name (non-valid characters &%$ etc)
//***************************************************************************
bool checkSaveOptions(String requestString) {

  String newSSID = "";
  String newPASSWORD = "";
  String newLocalName = "";
  String newTempSetting = "";
  uint8_t indexStart, indexEnd = 0;

  indexStart = requestString.indexOf("=");
  indexEnd   = requestString.indexOf("&", (indexStart + 1));
  if (indexStart == 255 || indexEnd == 255) {
    DEBUG_PRINTLN("Error.  Local Name not Found");
    return false;
  } else {
    newLocalName = requestString.substring((indexStart + 6), indexEnd);
    DEBUG_PRINT("Received LocalName: ");
    DEBUG_PRINTLN(newLocalName);
  }
  indexStart = requestString.indexOf("=", (indexStart + 1));
  indexEnd   = requestString.indexOf("&", (indexStart + 1));
  if (indexStart == 255 || indexEnd == 255) {
    DEBUG_PRINTLN("Error.  Temperature Setting Not Found");
    return false;
  } else {
    newTempSetting = requestString.substring((indexStart + 6), indexEnd);
    DEBUG_PRINT("Received TempSetting: ");
    DEBUG_PRINTLN(newTempSetting);
  }
  indexStart = requestString.indexOf("=", (indexStart + 1));
  indexEnd   = requestString.indexOf("&", (indexStart + 1));
  if (indexStart == 255 || indexEnd == 255) {
    DEBUG_PRINTLN("Error.  SSID Setting Not Found");
    return false;
  } else {
    newSSID = requestString.substring((indexStart + 6), indexEnd);
    DEBUG_PRINT("Received SSID Setting: ");
    DEBUG_PRINTLN(newSSID);
  }
  indexStart = requestString.indexOf("=", (indexStart + 1));
  indexEnd   = requestString.indexOf("&", (indexStart + 1));
  if (indexStart == 255 || indexEnd == 255) {
    DEBUG_PRINTLN("Error.  PASSWORD Setting Not Found");
    return false;
  } else {
    newPASSWORD = requestString.substring((indexStart + 6), indexEnd);
    DEBUG_PRINT("Received PASSWORD Setting: ");
    DEBUG_PRINTLN(newPASSWORD);
  }
  DEBUG_PRINTLN("Writing settings to EEPROM:");
  strcpy(EEPROMSettings.s_LCLversion, LCLversion);
  DEBUG_PRINT("Writing Version: "); DEBUG_PRINTLN(LCLversion);
  if (newSSID != "") {
    strcpy(EEPROMSettings.s_ssid, newSSID.c_str());
    DEBUG_PRINT("Writing SSID: "); DEBUG_PRINTLN(newSSID);
  }
  if (newPASSWORD != "") {
    strcpy(EEPROMSettings.s_password, newPASSWORD.c_str());
    DEBUG_PRINT("Writing PASSWORD: "); DEBUG_PRINTLN(newPASSWORD);
  }
  if (newLocalName != "") {
    strcpy(EEPROMSettings.s_localName, newLocalName.c_str());          // need to get newLocalName later from settings page
    DEBUG_PRINT("Writing Local Name: "); DEBUG_PRINTLN(newLocalName);
  }
  if (newTempSetting != "") {
    strcpy(EEPROMSettings.s_tempSetting, newTempSetting.c_str());
    DEBUG_PRINT("Writing Temperature Choice: "); DEBUG_PRINTLN(newTempSetting);
  }
  // TODO - add eeprom write and check of settings value here
  writeEEPROMSettingsStruct();
  return true;
}//end checkSaveOptions()
//***************************************************************************
// readEEPROMSettings()                                                     *
//***************************************************************************
void readEEPROMSettings() {
  uint8_t Validate = 0;

  EEPROM_readAnything(0, EEPROMSettings);
  DEBUG_PRINT("Version           : "); DEBUG_PRINTLN(EEPROMSettings.s_LCLversion);
  DEBUG_PRINT("SSID              : "); DEBUG_PRINTLN(EEPROMSettings.s_ssid);
  DEBUG_PRINT("Password          : "); DEBUG_PRINTLN(EEPROMSettings.s_password);
  DEBUG_PRINT("Local Name        : "); DEBUG_PRINTLN(EEPROMSettings.s_localName);
  DEBUG_PRINT("Temperature Choice: "); DEBUG_PRINTLN(EEPROMSettings.s_tempSetting);
  DEBUG_PRINTLN("END EEPROM Structure: ");
  Validate = strcmp(EEPROMSettings.s_LCLversion, LCLversion);
  if (Validate != 0) {
    Serial.println("EEPROM Failed Verification,  Setting default values");
  } else {
    strcpy(LCLversion, EEPROMSettings.s_LCLversion);
    strcpy(ssid, EEPROMSettings.s_ssid);
    strcpy(password, EEPROMSettings.s_password);
    strcpy(localName, EEPROMSettings.s_localName);
    strcpy(tempSetting, EEPROMSettings.s_tempSetting);
    Serial.println("EEPROM Verified, attempting to connect to network:");
    delay(100);
  }
}
//***************************************************************************
// writeEEPROMSettingsStruct()                                              *
//***************************************************************************
void writeEEPROMSettingsStruct() {

  int validated = 0;

  for (int i = 0; i < 512; i++) {
    EEPROM.write(i, 0);
  }

  EEPROM_writeAnything(0, EEPROMSettings);
  EEPROM.commit();
  delay(500);
  EEPROM_readAnything(0, EEPROMValidate);

  if (strcmp(EEPROMSettings.s_LCLversion, EEPROMValidate.s_LCLversion) != 0) {
    validated = 1;
    DEBUG_PRINTLN("EEPROM Write Failed Version");
  } else {
    DEBUG_PRINTLN(EEPROMValidate.s_LCLversion);
  }
  if (strcmp(EEPROMSettings.s_ssid, EEPROMValidate.s_ssid) != 0) {
    validated = 1;
    DEBUG_PRINTLN("EEPROM Write Failed SSID");
  } else {
    DEBUG_PRINTLN(EEPROMValidate.s_ssid);
  }
  if (strcmp(EEPROMSettings.s_password, EEPROMValidate.s_password) != 0) {
    validated = 1;
    DEBUG_PRINTLN("EEPROM Write Failed Password");
  } else {
    DEBUG_PRINTLN(EEPROMValidate.s_password);
  }
  if (strcmp(EEPROMSettings.s_localName, EEPROMValidate.s_localName) != 0) {
    validated = 1;
    DEBUG_PRINTLN("EEPROM Write Failed Local Name");
  } else {
    DEBUG_PRINTLN(EEPROMValidate.s_localName);
  }
  if (strcmp(EEPROMSettings.s_tempSetting, EEPROMValidate.s_tempSetting) != 0) {
    validated = 1;
    DEBUG_PRINTLN("EEPROM Write Failed Temperature Setting");
  } else {
    DEBUG_PRINTLN(EEPROMValidate.s_tempSetting);
  }
  DEBUG_PRINTLN("EEPROM Checked Successfully");
}
//***************************************************************************
// writeEEPROMSettingsStruct()                                              *
//***************************************************************************
void listSSIDs() {
  DEBUG_PRINTLN("SCANNING for available Networks");

  int n = WiFi.scanNetworks();

  DEBUG_PRINTLN("scan done");
  if (n == 0)
    DEBUG_PRINTLN("no networks found");
  else
  {
    DEBUG_PRINT(n);
    DEBUG_PRINTLN(" networks found");
    for (int i = 0; i < n; ++i)
    {
      // Print SSID and RSSI for each network found
      DEBUG_PRINT(i + 1);
      DEBUG_PRINT(": ");
      DEBUG_PRINT(WiFi.SSID(i));
      DEBUG_PRINT(" (");
      DEBUG_PRINT(WiFi.RSSI(i));
      DEBUG_PRINT(")");
      DEBUG_PRINTLN(WiFi.encryptionType(i));
      delay(10);
    }
  }
  // loop this to add lines to build entry for table and add tr tags (to separate rows)
  if (n > 10) {
    n = 10; // limit to showing only first 10 SSID's found
  }
  String tableLine = "";
  //  String tableLines[n]; // make global, easier
  for (int i = 0; i < 0; i++) { // Clear exising data
    tableLines[i] = "";
  }
  for (int i = 0; i < n; i++) {
    tableLine = "<tr><td>";
    tableLine += WiFi.SSID(i);
    tableLine += "</td><td>";
    tableLine += WiFi.RSSI(i);
    tableLine += "</td><td>";
    if (WiFi.encryptionType(i) == 2) {
      tableLine += "TKIP (WPA)";
    } else if (WiFi.encryptionType(i) == 4) {
      tableLine += "CCMP (WPA)";
    } else if (WiFi.encryptionType(i) == 5) {
      tableLine += "WEP)";
    } else if (WiFi.encryptionType(i) == 7) {
      tableLine += "NONE";
    } else if (WiFi.encryptionType(i) == 8) {
      tableLine += "AUTO";
    } else {
      tableLine += "UNKNOWN";
    }
    tableLine += "</td></tr>";
    tableLines[i] = tableLine;
  }
  tableLine = "";
  for (int i = 0; i < n; i++) {
    tableLine += tableLines[i];
  }
}
//***************************************************************************
// Setup Access Point                                                       *
//***************************************************************************
void setupAP() {

  WiFi.mode(WIFI_STA);
  WiFi.disconnect();
  delay(100);
  Serial.println("SCANNING for available Networks");
  int n = WiFi.scanNetworks();
  Serial.println("scan done");
  if (n == 0)
    Serial.println("no networks found");
  else
  {
    Serial.print(n);
    Serial.println(" networks found");
    for (int i = 0; i < n; ++i)
    {
      // Print SSID and RSSI for each network found
      Serial.print(i + 1);
      Serial.print(": ");
      Serial.print(WiFi.SSID(i));
      Serial.print(" (");
      Serial.print(WiFi.RSSI(i));
      Serial.print(")");
      Serial.println((WiFi.encryptionType(i) == ENC_TYPE_NONE) ? " " : "*");
      delay(10);
    }
  }
  Serial.println("Starting Access Point");
  WiFi.softAP(apSSID, apPASS);
  //  Serial.println("softap");
  //  Serial.println("");
  Serial.println("WiFi connected");
  Serial.print("Local IP: ");
  Serial.println(WiFi.localIP());
  Serial.print("SoftAP IP: ");
  Serial.println(WiFi.softAPIP());
  httpServer.begin();                                       // Start TCP (HTTP) server
  Serial.println("SOFTAP Server started");
  digitalWrite(BLUELED, LOW);
  digitalWrite(GREENLED, HIGH);
}
//***************************************************************************
// Query micro for Temperature sensor information                           *
//***************************************************************************
void getConfig() {
  unsigned long startMillis = millis();
  unsigned long currentMillis = startMillis;
  String inSerial = "";

  Serial.print("Starting getConfig at t: ");
  Serial.println(millis());
  while (Serial.available()) {  // Clear Rx Buffer
    Serial.read();
  }
  Serial.println("Q");        // Query host micro for time, date and temperature data
  while ((currentMillis - startMillis) < 1000) {
    //wait
    if (Serial.available() > 0) {
      currentMillis = currentMillis + 1000;
    } else {
      currentMillis = millis();
    }
  }
  if (Serial.available() > 0) {
    Serial.readBytesUntil('\r', rxBuffer, 64);
    inSerial = String(rxBuffer);
    switch (inSerial[0]) {
      case 'T':                 // Temperature information - (SIGN)ttt.t where sign is - for negative temperature, blank otherwise
        currentTemperature = inSerial.substring(2, 8);
        Serial.print("CurrentTemperature ");
        Serial.println(currentTemperature);
        break;
      default:                                // Other cases possible later (? alarm zone timing ?)
        Serial.println("Temp Data Not Found ");
        Serial.print("String Recieved was: ");
        Serial.println(inSerial);
        break;
    }
  }
  else {
    Serial.println("Error - Timeout.  No Query data recieved");
  }
}

